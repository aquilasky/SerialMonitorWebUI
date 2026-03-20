<script setup lang="ts">
import { computed } from 'vue'
import { useSerialStore } from '../stores/serial'

const store = useSerialStore()

const statusText = computed(() => {
  if (store.connecting) return 'Connecting...'
  if (store.connected) return `Connected to ${store.config.port}`
  return 'Disconnected'
})

const txCount = computed(() => {
  return store.dataEntries.filter(e => e.isSend).length
})

const rxCount = computed(() => {
  return store.dataEntries.filter(e => !e.isSend).length
})
</script>

<template>
  <footer class="status-bar">
    <div class="status-left">
      <div class="status-indicator">
        <div class="status-dot" :class="{ connected: store.connected, connecting: store.connecting }"></div>
        <span class="status-text">{{ statusText }}</span>
      </div>
      <div class="status-config" v-if="store.connected">
        <span>{{ store.config.baudrate }}</span>
        <span class="separator">-</span>
        <span>{{ store.config.databits }}</span>
        <span class="separator">-</span>
        <span>{{ store.config.parity }}</span>
        <span class="separator">-</span>
        <span>{{ store.config.stopbits }}</span>
      </div>
    </div>

    <div class="status-right">
      <div class="stat-item">
        <span class="stat-label">TX:</span>
        <span class="stat-value text-accent">{{ txCount }}</span>
      </div>
      <div class="stat-item">
        <span class="stat-label">RX:</span>
        <span class="stat-value text-success">{{ rxCount }}</span>
      </div>
      <div class="version">SerialMonitor v1.0.0</div>
    </div>
  </footer>
</template>

<style lang="scss" scoped>
.status-bar {
  background: var(--bg-secondary);
  padding: 10px 24px;
  display: flex;
  align-items: center;
  justify-content: space-between;
  border-top: 1px solid var(--border);
  font-size: 13px;
  color: var(--text-secondary);
  animation: slideUp 0.4s ease;
}

@keyframes slideUp {
  from {
    opacity: 0;
    transform: translateY(20px);
  }
  to {
    opacity: 1;
    transform: translateY(0);
  }
}

.status-left {
  display: flex;
  align-items: center;
  gap: 20px;
}

.status-indicator {
  display: flex;
  align-items: center;
  gap: 8px;
}

.status-dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  background: var(--danger);
  transition: all 0.3s ease;

  &.connected {
    background: var(--success);
    animation: pulse 2s infinite;
  }

  &.connecting {
    background: var(--warning);
    animation: pulse 0.5s infinite;
  }
}

@keyframes pulse {
  0%, 100% { opacity: 1; }
  50% { opacity: 0.5; }
}

.status-config {
  display: flex;
  align-items: center;
  gap: 4px;
  font-family: 'JetBrains Mono', monospace;
  font-size: 12px;

  .separator {
    color: var(--border);
  }
}

.status-right {
  display: flex;
  align-items: center;
  gap: 20px;
}

.stat-item {
  display: flex;
  align-items: center;
  gap: 4px;
}

.stat-label {
  color: var(--text-secondary);
}

.stat-value {
  font-family: 'JetBrains Mono', monospace;
  font-weight: 600;
}

.version {
  padding-left: 16px;
  border-left: 1px solid var(--border);
}
</style>
