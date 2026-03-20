<script setup lang="ts">
import { onMounted, ref } from 'vue'
import { useSerialStore } from './stores/serial'
import AppHeader from './components/AppHeader.vue'
import ConnectionPanel from './components/ConnectionPanel.vue'
import DataDisplay from './components/DataDisplay.vue'
import SendPanel from './components/SendPanel.vue'
import StatusBar from './components/StatusBar.vue'

const store = useSerialStore()
const initialized = ref(false)

onMounted(async () => {
  await store.loadConfig()
  await store.fetchPorts()
  await store.checkStatus()
  initialized.value = true
})
</script>

<template>
  <div class="app-container" v-if="initialized">
    <AppHeader />
    <main class="main-content">
      <aside class="sidebar">
        <ConnectionPanel />
      </aside>
      <section class="data-area">
        <DataDisplay />
        <SendPanel />
      </section>
    </main>
    <StatusBar />
  </div>
  <div v-else class="loading-container">
    <div class="loading-spinner"></div>
    <p>Loading...</p>
  </div>
</template>

<style lang="scss" scoped>
.app-container {
  height: 100%;
  display: flex;
  flex-direction: column;
  animation: fadeIn 0.5s ease;
}

.main-content {
  flex: 1;
  display: flex;
  padding: 20px;
  gap: 20px;
  overflow: hidden;
}

.sidebar {
  width: 300px;
  flex-shrink: 0;
  display: flex;
  flex-direction: column;
  gap: 16px;
  overflow-y: auto;
}

.data-area {
  flex: 1;
  display: flex;
  flex-direction: column;
  gap: 16px;
  min-width: 0;
}

.loading-container {
  height: 100vh;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  gap: 16px;
  color: var(--text-secondary);
}

.loading-spinner {
  width: 40px;
  height: 40px;
  border: 3px solid var(--border);
  border-top-color: var(--accent);
  border-radius: 50%;
  animation: spin 1s linear infinite;
}

@keyframes spin {
  to { transform: rotate(360deg); }
}

@keyframes fadeIn {
  from { opacity: 0; }
  to { opacity: 1; }
}
</style>
