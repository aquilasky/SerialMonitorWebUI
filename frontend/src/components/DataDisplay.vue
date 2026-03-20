<script setup lang="ts">
import { ref, computed, watch, nextTick } from 'vue'
import { useSerialStore } from '../stores/serial'

const store = useSerialStore()
const dataDisplayRef = ref<HTMLElement | null>(null)

const formatOptions = [
  { label: 'ASCII', value: 'ascii' },
  { label: 'Hex', value: 'hex' },
  { label: 'Decimal', value: 'decimal' },
  { label: 'Binary', value: 'binary' },
  { label: 'Mixed', value: 'mixed' }
]

const encodingOptions = [
  { label: 'UTF-8', value: 'utf-8' },
  { label: 'GBK', value: 'gbk' },
  { label: 'GB2312', value: 'gb2312' },
  { label: 'ASCII', value: 'ascii' }
]

function formatByte(byte: number): string {
  switch (store.config.format) {
    case 'hex':
      return byte.toString(16).padStart(2, '0').toUpperCase()
    case 'decimal':
      return byte.toString()
    case 'binary':
      return byte.toString(2).padStart(8, '0')
    case 'ascii':
      return byte >= 32 && byte < 127 ? String.fromCharCode(byte) : '.'
    case 'mixed':
      const hex = byte.toString(16).padStart(2, '0').toUpperCase()
      const ascii = byte >= 32 && byte < 127 ? String.fromCharCode(byte) : '.'
      return `${hex} ${ascii}`
    default:
      return byte.toString(16).padStart(2, '0').toUpperCase()
  }
}

function formatData(data: string): { html: string; raw: string } {
  const bytes: number[] = []
  for (let i = 0; i < data.length; i++) {
    bytes.push(data.charCodeAt(i))
  }

  let html = ''
  let raw = ''

  if (store.config.format === 'mixed') {
    for (let i = 0; i < bytes.length; i++) {
      const hex = bytes[i].toString(16).padStart(2, '0').toUpperCase()
      const ascii = bytes[i] >= 32 && bytes[i] < 127 ? String.fromCharCode(bytes[i]) : '.'
      html += `<span class="hex-format">${hex}</span> <span class="char">${ascii}</span> `
      raw += hex + ' '
    }
  } else if (store.config.format === 'ascii') {
    for (let i = 0; i < bytes.length; i++) {
      const char = bytes[i] >= 32 && bytes[i] < 127 ? String.fromCharCode(bytes[i]) : '.'
      html += `<span class="char">${char === ' ' ? '&nbsp;' : char}</span>`
      raw += String.fromCharCode(bytes[i])
    }
  } else {
    for (let i = 0; i < bytes.length; i++) {
      html += `<span class="hex-format">${formatByte(bytes[i])}</span> `
      raw += formatByte(bytes[i]) + ' '
    }
  }

  return { html, raw }
}

function handleFormatChange() {
  store.saveConfig()
}

function handleEncodingChange() {
  store.saveConfig()
}

function handleClear() {
  store.clearData()
}

watch(() => store.dataEntries.length, () => {
  if (store.config.autoscroll) {
    nextTick(() => {
      if (dataDisplayRef.value) {
        dataDisplayRef.value.scrollTop = dataDisplayRef.value.scrollHeight
      }
    })
  }
})
</script>

<template>
  <div class="data-display">
    <div class="display-header">
      <div class="header-left">
        <span class="display-title">Data Monitor</span>
        <el-select
          v-model="store.config.format"
          size="small"
          style="width: 120px; margin-left: 16px"
          @change="handleFormatChange"
        >
          <el-option
            v-for="opt in formatOptions"
            :key="opt.value"
            :label="opt.label"
            :value="opt.value"
          />
        </el-select>
        <el-select
          v-model="store.config.encoding"
          size="small"
          style="width: 100px; margin-left: 8px"
          @change="handleEncodingChange"
        >
          <el-option
            v-for="opt in encodingOptions"
            :key="opt.value"
            :label="opt.label"
            :value="opt.value"
          />
        </el-select>
      </div>
      <div class="header-right">
        <el-checkbox
          v-model="store.config.timestamp"
          size="small"
          @change="store.saveConfig"
        >
          Timestamp
        </el-checkbox>
        <el-checkbox
          v-model="store.config.autoscroll"
          size="small"
          @change="store.saveConfig"
        >
          Auto Scroll
        </el-checkbox>
        <el-button size="small" @click="handleClear">Clear</el-button>
      </div>
    </div>

    <div class="display-content monospace" ref="dataDisplayRef">
      <div
        v-for="entry in store.dataEntries"
        :key="entry.id"
        class="data-line"
        :class="{ send: entry.isSend }"
      >
        <span v-if="store.config.timestamp" class="timestamp">[{{ entry.timestamp }}]</span>
        <span class="direction">{{ entry.isSend ? '>' : '<' }}</span>
        <span v-html="formatData(entry.data).html"></span>
      </div>
      <div v-if="store.dataEntries.length === 0" class="empty-state">
        <span>No data yet. Connect to a port to start receiving.</span>
      </div>
    </div>
  </div>
</template>

<style lang="scss" scoped>
.data-display {
  flex: 1;
  display: flex;
  flex-direction: column;
  min-height: 0;
  animation: slideInUp 0.4s ease;
}

.display-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 12px 16px;
  border-bottom: 1px solid var(--border);
  background: var(--bg-secondary);
  border-radius: var(--radius-lg) var(--radius-lg) 0 0;
}

.header-left {
  display: flex;
  align-items: center;
}

.header-right {
  display: flex;
  align-items: center;
  gap: 16px;
}

.display-title {
  font-size: 14px;
  font-weight: 600;
  color: var(--text-primary);
}

.display-content {
  flex: 1;
  padding: 12px;
  overflow-y: auto;
  background: var(--bg-secondary);
  border-radius: 0 0 var(--radius-lg) var(--radius-lg);
  border: 1px solid var(--border);
  border-top: none;
  transition: border-color 0.3s ease;

  &:hover {
    border-color: var(--accent);
  }
}

.data-line {
  padding: 4px 8px;
  border-radius: 4px;
  animation: flash 0.3s ease;
  line-height: 1.6;

  &.send {
    background: rgba(74, 158, 255, 0.08);
  }

  &:hover {
    background: rgba(74, 158, 255, 0.05);
  }
}

.timestamp {
  color: var(--text-secondary);
  font-size: 11px;
  margin-right: 8px;
}

.direction {
  font-weight: bold;
  margin-right: 8px;
  color: var(--accent);
}

.send .direction {
  color: var(--warning);
}

.hex-format {
  color: var(--warning);
}

.char {
  color: var(--success);
}

.empty-state {
  display: flex;
  align-items: center;
  justify-content: center;
  height: 100%;
  color: var(--text-secondary);
  font-size: 14px;
}
</style>
