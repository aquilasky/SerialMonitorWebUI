<script setup lang="ts">
import { ref } from 'vue'
import { useSerialStore } from '../stores/serial'

const store = useSerialStore()
const inputData = ref('')

const lineEndingOptions = [
  { label: 'CR+LF', value: 'crlf' },
  { label: 'CR', value: 'cr' },
  { label: 'LF', value: 'lf' },
  { label: 'None', value: 'none' }
]

async function handleSend() {
  if (!inputData.value) return
  await store.sendData(inputData.value)
  inputData.value = ''
}

function handleKeydown(e: KeyboardEvent) {
  if (e.key === 'Enter' && !e.shiftKey) {
    e.preventDefault()
    handleSend()
  }
}

function handleHistorySelect(cmd: string) {
  inputData.value = cmd
}
</script>

<template>
  <div class="send-panel">
    <div class="send-input-area">
      <el-input
        v-model="inputData"
        type="textarea"
        :rows="3"
        placeholder="Enter data to send..."
        @keydown="handleKeydown"
        resize="none"
        :disabled="!store.connected"
      />

      <div class="send-controls">
        <div class="control-left">
          <el-select
            v-model="store.config.lineEnding"
            size="small"
            style="width: 100px"
            @change="store.saveConfig"
          >
            <el-option
              v-for="opt in lineEndingOptions"
              :key="opt.value"
              :label="opt.label"
              :value="opt.value"
            />
          </el-select>
          <el-checkbox
            v-model="store.config.hexMode"
            size="small"
            @change="store.saveConfig"
          >
            Hex Mode
          </el-checkbox>
        </div>

        <div class="control-right">
          <el-button
            type="primary"
            @click="handleSend"
            :disabled="!store.connected || !inputData"
          >
            Send
          </el-button>
        </div>
      </div>
    </div>

    <div v-if="store.sendHistory.length > 0" class="history-panel">
      <span class="history-title">Quick Commands:</span>
      <div class="history-items">
        <el-tag
          v-for="(cmd, index) in store.sendHistory.slice(0, 10)"
          :key="index"
          size="small"
          class="history-tag"
          @click="handleHistorySelect(cmd)"
        >
          {{ cmd.length > 20 ? cmd.slice(0, 20) + '...' : cmd }}
        </el-tag>
      </div>
    </div>
  </div>
</template>

<style lang="scss" scoped>
.send-panel {
  animation: slideInUp 0.4s ease;
  animation-delay: 0.2s;
  animation-fill-mode: both;
}

.send-input-area {
  background: var(--bg-secondary);
  border-radius: var(--radius-lg);
  border: 1px solid var(--border);
  padding: 16px;
  transition: all 0.3s ease;

  &:focus-within {
    border-color: var(--accent);
    box-shadow: 0 4px 20px rgba(74, 158, 255, 0.1);
  }
}

.send-controls {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-top: 12px;
}

.control-left {
  display: flex;
  align-items: center;
  gap: 16px;
}

.control-right {
  display: flex;
  gap: 8px;
}

.history-panel {
  margin-top: 12px;
  padding: 12px;
  background: var(--bg-secondary);
  border-radius: var(--radius-md);
  border: 1px solid var(--border);
}

.history-title {
  font-size: 12px;
  color: var(--text-secondary);
  margin-bottom: 8px;
  display: block;
}

.history-items {
  display: flex;
  flex-wrap: wrap;
  gap: 6px;
}

.history-tag {
  cursor: pointer;
  transition: all 0.2s ease;

  &:hover {
    transform: scale(1.05);
  }
}
</style>
