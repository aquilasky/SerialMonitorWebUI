<script setup lang="ts">
import { ref } from 'vue'
import { ElMessage } from 'element-plus'
import { useSerialStore } from '../stores/serial'

const store = useSerialStore()

const baudrates = [
  300, 1200, 2400, 4800, 9600, 19200, 38400, 57600,
  115200, 230400, 460800, 921600
]

const databitsOptions = [5, 6, 7, 8]
const parityOptions = [
  { label: 'None', value: 'none' },
  { label: 'Odd', value: 'odd' },
  { label: 'Even', value: 'even' },
  { label: 'Mark', value: 'mark' },
  { label: 'Space', value: 'space' }
]
const stopbitsOptions = [
  { label: '1', value: 1 },
  { label: '1.5', value: 1.5 },
  { label: '2', value: 2 }
]
const flowOptions = [
  { label: 'None', value: 'none' },
  { label: 'Hardware', value: 'hardware' },
  { label: 'Software', value: 'software' }
]

async function handleConnect() {
  try {
    await store.connect()
    ElMessage.success(`Connected to ${store.config.port}`)
  } catch (e: any) {
    ElMessage.error(e.message || 'Connection failed')
  }
}

async function handleDisconnect() {
  try {
    await store.disconnect()
    ElMessage.info('Disconnected')
  } catch (e: any) {
    ElMessage.error(e.message || 'Disconnect failed')
  }
}

async function handleRefresh() {
  await store.fetchPorts()
}
</script>

<template>
  <div class="connection-panel panel">
    <div class="panel-header">
      <span class="panel-title">Connection</span>
      <el-button size="small" :icon="Refresh" @click="handleRefresh" circle />
    </div>

    <el-form label-position="top" size="default">
      <el-form-item label="Port">
        <el-select
          v-model="store.config.port"
          placeholder="Select port"
          style="width: 100%"
          :disabled="store.connected"
        >
          <el-option
            v-for="p in store.ports"
            :key="p.name"
            :label="p.name"
            :value="p.name"
          >
            <span>{{ p.name }}</span>
            <span class="port-desc" v-if="p.description">{{ p.description }}</span>
          </el-option>
        </el-select>
      </el-form-item>

      <el-form-item label="Baud Rate">
        <el-select
          v-model="store.config.baudrate"
          placeholder="Select baud rate"
          style="width: 100%"
          :disabled="store.connected"
        >
          <el-option
            v-for="rate in baudrates"
            :key="rate"
            :label="rate.toString()"
            :value="rate"
          />
        </el-select>
      </el-form-item>

      <el-form-item label="Data Bits">
        <el-select
          v-model="store.config.databits"
          style="width: 100%"
          :disabled="store.connected"
        >
          <el-option
            v-for="bits in databitsOptions"
            :key="bits"
            :label="bits.toString()"
            :value="bits"
          />
        </el-select>
      </el-form-item>

      <el-form-item label="Parity">
        <el-select
          v-model="store.config.parity"
          style="width: 100%"
          :disabled="store.connected"
        >
          <el-option
            v-for="opt in parityOptions"
            :key="opt.value"
            :label="opt.label"
            :value="opt.value"
          />
        </el-select>
      </el-form-item>

      <el-form-item label="Stop Bits">
        <el-select
          v-model="store.config.stopbits"
          style="width: 100%"
          :disabled="store.connected"
        >
          <el-option
            v-for="opt in stopbitsOptions"
            :key="opt.value"
            :label="opt.label"
            :value="opt.value"
          />
        </el-select>
      </el-form-item>

      <el-form-item label="Flow Control">
        <el-select
          v-model="store.config.flowcontrol"
          style="width: 100%"
          :disabled="store.connected"
        >
          <el-option
            v-for="opt in flowOptions"
            :key="opt.value"
            :label="opt.label"
            :value="opt.value"
          />
        </el-select>
      </el-form-item>
    </el-form>

    <div class="connection-actions">
      <el-button
        type="primary"
        :disabled="store.connected || store.connecting"
        :loading="store.connecting"
        @click="handleConnect"
        class="connect-btn"
      >
        {{ store.connecting ? 'Connecting...' : 'Connect' }}
      </el-button>
      <el-button
        @click="handleDisconnect"
        :disabled="!store.connected"
      >
        Disconnect
      </el-button>
    </div>
  </div>
</template>

<style lang="scss" scoped>
.connection-panel {
  animation-delay: 0.1s;
}

.panel-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: 16px;
}

.panel-title {
  font-size: 14px;
  font-weight: 600;
  color: var(--text-secondary);
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

.port-desc {
  float: right;
  color: var(--text-secondary);
  font-size: 12px;
}

.connection-actions {
  display: flex;
  gap: 8px;
  margin-top: 20px;
}

.connect-btn {
  flex: 1;
}
</style>
