import { defineStore } from 'pinia'
import { ref, computed } from 'vue'

export interface PortInfo {
  name: string
  description: string
  hardwareId: string
}

export interface SerialConfig {
  port: string
  baudrate: number
  databits: number
  parity: string
  stopbits: number
  flowcontrol: string
  format: string
  encoding: string
  timestamp: boolean
  autoscroll: boolean
  lineEnding: string
  hexMode: boolean
  repeatEnabled: boolean
  repeatInterval: number
  theme: string
  language: string
}

export interface DataEntry {
  id: number
  data: string
  timestamp: string
  isSend: boolean
}

export const useSerialStore = defineStore('serial', () => {
  const ports = ref<PortInfo[]>([])
  const connected = ref(false)
  const connecting = ref(false)

  const config = ref<SerialConfig>({
    port: 'COM3',
    baudrate: 115200,
    databits: 8,
    parity: 'none',
    stopbits: 1,
    flowcontrol: 'none',
    format: 'hex',
    encoding: 'utf-8',
    timestamp: true,
    autoscroll: true,
    lineEnding: 'crlf',
    hexMode: false,
    repeatEnabled: false,
    repeatInterval: 1000,
    theme: 'dark-blue',
    language: 'zh-CN'
  })

  const dataEntries = ref<DataEntry[]>([])
  const nextId = ref(1)

  const sendHistory = ref<string[]>([])
  const maxHistorySize = 50

  let ws: WebSocket | null = null
  let reconnectTimer: number | null = null

  const themes: Record<string, Record<string, string>> = {
    'dark-blue': {
      '--bg-primary': '#1a1d29',
      '--bg-secondary': '#242836',
      '--bg-tertiary': '#2d3348',
      '--text-primary': '#e8eaed',
      '--text-secondary': '#9aa0a6',
      '--accent': '#4a9eff',
      '--accent-hover': '#6bb3ff',
      '--success': '#34d399',
      '--warning': '#fbbf24',
      '--danger': '#ef4444',
      '--border': '#3d4556'
    },
    'carbon-black': {
      '--bg-primary': '#0d0d0d',
      '--bg-secondary': '#1a1a1a',
      '--bg-tertiary': '#262626',
      '--text-primary': '#e0e0e0',
      '--text-secondary': '#808080',
      '--accent': '#00d4ff',
      '--accent-hover': '#33dfff',
      '--success': '#00ff88',
      '--warning': '#ffaa00',
      '--danger': '#ff4444',
      '--border': '#333333'
    },
    'matrix-green': {
      '--bg-primary': '#0a1f0a',
      '--bg-secondary': '#0f2f0f',
      '--bg-tertiary': '#1a3f1a',
      '--text-primary': '#00ff00',
      '--text-secondary': '#00aa00',
      '--accent': '#00ff00',
      '--accent-hover': '#33ff33',
      '--success': '#00ff00',
      '--warning': '#aaff00',
      '--danger': '#ff3300',
      '--border': '#2a5f2a'
    },
    'daylight-white': {
      '--bg-primary': '#f5f5f5',
      '--bg-secondary': '#ffffff',
      '--bg-tertiary': '#e8e8e8',
      '--text-primary': '#1a1a1a',
      '--text-secondary': '#666666',
      '--accent': '#2563eb',
      '--accent-hover': '#1d4ed8',
      '--success': '#16a34a',
      '--warning': '#d97706',
      '--danger': '#dc2626',
      '--border': '#d1d5db'
    },
    'violet': {
      '--bg-primary': '#1a1625',
      '--bg-secondary': '#251f33',
      '--bg-tertiary': '#352a45',
      '--text-primary': '#e8e8f0',
      '--text-secondary': '#9aa0b0',
      '--accent': '#9b6dff',
      '--accent-hover': '#b08dff',
      '--success': '#34d399',
      '--warning': '#fbbf24',
      '--danger': '#ef4444',
      '--border': '#4a4260'
    },
    'sunset-orange': {
      '--bg-primary': '#2a1f1a',
      '--bg-secondary': '#3a2a22',
      '--bg-tertiary': '#4a3530',
      '--text-primary': '#f0e8e4',
      '--text-secondary': '#b0a098',
      '--accent': '#ff8c42',
      '--accent-hover': '#ffa666',
      '--success': '#34d399',
      '--warning': '#fbbf24',
      '--danger': '#ef4444',
      '--border': '#5a4540'
    }
  }

  const currentTheme = computed(() => themes[config.value.theme] || themes['dark-blue'])

  async function fetchPorts() {
    try {
      const res = await fetch('/api/ports')
      const data = await res.json()
      if (data.success) {
        ports.value = data.data
      }
    } catch (e) {
      console.error('Failed to fetch ports:', e)
    }
  }

  async function loadConfig() {
    try {
      const res = await fetch('/api/config')
      const data = await res.json()
      if (data.success) {
        Object.assign(config.value, data.data)
        applyTheme(config.value.theme)
      }
    } catch (e) {
      console.error('Failed to load config:', e)
    }
  }

  async function saveConfig() {
    try {
      const params = new URLSearchParams(config.value as any)
      await fetch('/api/config?' + params, { method: 'POST' })
    } catch (e) {
      console.error('Failed to save config:', e)
    }
  }

  function applyTheme(themeName: string) {
    const theme = themes[themeName]
    if (theme) {
      Object.entries(theme).forEach(([key, value]) => {
        document.documentElement.style.setProperty(key, value)
      })
    }
  }

  function setTheme(themeName: string) {
    config.value.theme = themeName
    applyTheme(themeName)
    saveConfig()
  }

  async function connect() {
    if (connected.value || connecting.value) return

    connecting.value = true
    try {
      const params = new URLSearchParams({
        port: config.value.port,
        baudrate: config.value.baudrate.toString(),
        databits: config.value.databits.toString(),
        parity: config.value.parity,
        stopbits: config.value.stopbits.toString(),
        flowcontrol: config.value.flowcontrol
      })
      const res = await fetch('/api/connect?' + params)
      const data = await res.json()
      if (data.success) {
        connected.value = true
        connectWebSocket()
        saveConfig()
      } else {
        throw new Error(data.error || 'Connection failed')
      }
    } catch (e: any) {
      console.error('Connection failed:', e)
      throw e
    } finally {
      connecting.value = false
    }
  }

  async function disconnect() {
    try {
      await fetch('/api/disconnect')
      connected.value = false
      if (ws) {
        ws.close()
        ws = null
      }
      if (reconnectTimer) {
        clearTimeout(reconnectTimer)
        reconnectTimer = null
      }
    } catch (e) {
      console.error('Disconnect error:', e)
    }
  }

  function connectWebSocket() {
    const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:'
    ws = new WebSocket(protocol + '//' + window.location.host + '/ws')

    ws.onmessage = (event) => {
      if (event.data[0] === '\x81') {
        try {
          const jsonStr = event.data.substring(1)
          const msg = JSON.parse(jsonStr)
          if (msg.type === 'data') {
            addDataEntry(msg.data, msg.timestamp, false)
          }
        } catch (e) {
          console.error('WS message parse error:', e)
        }
      }
    }

    ws.onclose = () => {
      if (connected.value) {
        reconnectTimer = window.setTimeout(connectWebSocket, 1000)
      }
    }

    ws.onerror = () => {
      connected.value = false
    }
  }

  function addDataEntry(data: string, timestamp: string, isSend: boolean) {
    dataEntries.value.push({
      id: nextId.value++,
      data,
      timestamp,
      isSend
    })

    if (dataEntries.value.length > 10000) {
      dataEntries.value = dataEntries.value.slice(-5000)
    }
  }

  async function sendData(data: string) {
    if (!connected.value || !data) return

    let dataToSend = data
    if (config.value.lineEnding !== 'none') {
      if (config.value.lineEnding === 'crlf') dataToSend += '\r\n'
      else if (config.value.lineEnding === 'cr') dataToSend += '\r'
      else if (config.value.lineEnding === 'lf') dataToSend += '\n'
    }

    try {
      const params = new URLSearchParams({
        data: config.value.hexMode ? dataToSend : dataToSend,
        hexMode: config.value.hexMode ? 'true' : 'false'
      })
      await fetch('/api/send?' + params)
      addDataEntry(dataToSend, new Date().toLocaleTimeString(), true)

      if (!sendHistory.value.includes(data)) {
        sendHistory.value.unshift(data)
        if (sendHistory.value.length > maxHistorySize) {
          sendHistory.value.pop()
        }
      }
    } catch (e) {
      console.error('Send error:', e)
    }
  }

  function clearData() {
    dataEntries.value = []
  }

  async function checkStatus() {
    try {
      const res = await fetch('/api/status')
      const data = await res.json()
      if (data.connected && !connected.value) {
        connected.value = true
        connectWebSocket()
      }
    } catch (e) {
      console.error('Status check error:', e)
    }
  }

  return {
    ports,
    connected,
    connecting,
    config,
    dataEntries,
    sendHistory,
    themes,
    currentTheme,
    fetchPorts,
    loadConfig,
    saveConfig,
    applyTheme,
    setTheme,
    connect,
    disconnect,
    sendData,
    clearData,
    checkStatus
  }
})
