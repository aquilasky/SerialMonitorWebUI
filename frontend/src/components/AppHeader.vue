<script setup lang="ts">
import { useSerialStore } from '../stores/serial'

const store = useSerialStore()
</script>

<template>
  <header class="app-header">
    <div class="header-left">
      <div class="logo">
        <svg width="32" height="32" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
          <path d="M12 2L2 7l10 5 10-5-10-5z"/>
          <path d="M2 17l10 5 10-5"/>
          <path d="M2 12l10 5 10-5"/>
        </svg>
      </div>
      <h1 class="title">SerialMonitor</h1>
      <span class="subtitle">WebUI Serial Port Assistant</span>
    </div>

    <div class="header-right">
      <div class="theme-selector">
        <span class="theme-label">Theme:</span>
        <div class="theme-options">
          <div
            v-for="(theme, key) in store.themes"
            :key="key"
            class="theme-option"
            :class="{ active: store.config.theme === key }"
            :style="{ background: theme['--bg-tertiary'] }"
            :title="key"
            @click="store.setTheme(key)"
          >
            <div
              class="theme-color"
              :style="{
                background: theme['--accent'],
                borderColor: store.config.theme === key ? theme['--text-primary'] : 'transparent'
              }"
            ></div>
          </div>
        </div>
      </div>
    </div>
  </header>
</template>

<style lang="scss" scoped>
.app-header {
  background: var(--bg-secondary);
  padding: 16px 24px;
  display: flex;
  align-items: center;
  justify-content: space-between;
  border-bottom: 1px solid var(--border);
  animation: slideDown 0.4s ease;
}

@keyframes slideDown {
  from {
    opacity: 0;
    transform: translateY(-20px);
  }
  to {
    opacity: 1;
    transform: translateY(0);
  }
}

.header-left {
  display: flex;
  align-items: center;
  gap: 12px;
}

.logo {
  color: var(--accent);
  display: flex;
  align-items: center;
}

.title {
  font-size: 20px;
  font-weight: 600;
  color: var(--text-primary);
}

.subtitle {
  font-size: 13px;
  color: var(--text-secondary);
  padding-left: 12px;
  border-left: 1px solid var(--border);
}

.header-right {
  display: flex;
  align-items: center;
  gap: 24px;
}

.theme-selector {
  display: flex;
  align-items: center;
  gap: 12px;
}

.theme-label {
  font-size: 13px;
  color: var(--text-secondary);
}

.theme-options {
  display: flex;
  gap: 8px;
}

.theme-option {
  width: 32px;
  height: 32px;
  border-radius: 8px;
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  border: 2px solid transparent;
  transition: all 0.2s ease;

  &:hover {
    transform: scale(1.1);
  }

  &.active {
    border-color: var(--text-primary);
  }
}

.theme-color {
  width: 18px;
  height: 18px;
  border-radius: 4px;
  border: 2px solid transparent;
  transition: border-color 0.2s ease;
}
</style>
