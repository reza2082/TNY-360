<template>
    <div class="p-4">
        <div class="flex space-x-4">
            <div class="p-2">
                <h2 class="text-xl font-semibold"> CPU Usage ({{ Math.round(stats?.temp_c * 10) / 10 }}°C) </h2>
                <div class="flex space-x-8 p-2">
                    <div>
                        <p>Core 0</p>
                        <BarChart class="w-32"
                            :nbars="10" direction="vertical" :size="2" v-model="stats.cpu_usage.core0_percent.value"
                        />
                        <p class="text-center w-full"> {{Math.round(stats?.cpu_usage.core0 * 100)}} % </p>
                    </div>
                    <div>
                        <p>Core 1</p>
                        <BarChart class="w-32"
                            :nbars="10" direction="vertical" :size="2" v-model="stats.cpu_usage.core1_percent.value"
                        />
                        <p class="text-center w-full"> {{Math.round(stats?.cpu_usage.core1 * 100)}} % </p>
                    </div>
                </div>
            </div>
            <div class="p-2">
                <h2 class="text-xl font-semibold"> RAM Usage </h2>
                <div class="flex space-x-8 p-2">
                    <div>
                        <p>Internal</p>
                        <BarChart class="w-32"
                            :nbars="10" direction="vertical" :size="2" v-model="stats.ram_usage.internal_percent.value"
                        />
                        <p class="text-center w-full"> {{formatRAM(stats?.ram_usage.internal_used)}} / {{ formatRAM(stats?.ram_usage.internal_total) }} </p>
                    </div>
                    <div>
                        <p>PSRAM</p>
                        <BarChart class="w-32"
                            :nbars="10" direction="vertical" :size="2" v-model="stats.ram_usage.psram_percent.value"
                        />
                        <p class="text-center w-full"> {{formatRAM(stats?.ram_usage.psram_used)}} / {{ formatRAM(stats?.ram_usage.psram_total) }} </p>
                    </div>
                </div>
            </div>
        </div>
    </div>
</template>

<script lang="ts" setup>
import { useTNY360 } from '~/composables/TNY360';

const tny = useTNY360();

const stats = {
    temp_c: 0,
    cpu_usage: {
        core0: 0,
        core0_percent: ref(0),
        core1: 0,
        core1_percent: ref(0),
    },
    ram_usage: {
        internal_total: 0,
        internal_used: 0,
        internal_percent: ref(0),
        psram_total: 0,
        psram_used: 0,
        psram_percent: ref(0),
    }
}

let pollingInterval : number | null = null;
onMounted(() => {
    pollingInterval = setInterval(async  () => {
        try {
            const res = await tny.value?.system.getStatistics();
            if (res) {
                stats.temp_c = res.temp_c;
                stats.cpu_usage.core0 = res.cpu_usage.core0;
                stats.cpu_usage.core1 = res.cpu_usage.core1;
                stats.ram_usage.internal_used = res.ram_usage.internal_used;
                stats.ram_usage.internal_total = res.ram_usage.internal_total;
                stats.ram_usage.psram_used = res.ram_usage.psram_used;
                stats.ram_usage.psram_total = res.ram_usage.psram_total;

                stats.cpu_usage.core0_percent.value = res.cpu_usage.core0;
                stats.cpu_usage.core1_percent.value = res.cpu_usage.core1;
                stats.ram_usage.internal_percent.value = res.ram_usage.internal_used / res.ram_usage.internal_total;
                stats.ram_usage.psram_percent.value = res.ram_usage.psram_used / res.ram_usage.psram_total;
            }
        } catch (error) {
            console.error('Error fetching statistics:', error);
        }
    }, 1000);
});

onUnmounted(() => {
    if (pollingInterval) {
        clearInterval(pollingInterval);
    }
});

function formatRAM(bytes: number): string {
    if (bytes < 1024) return bytes + ' B';
    else if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(0) + ' KB';
    else if (bytes < 1024 * 1024 * 1024) return (bytes / (1024 * 1024)).toFixed(0) + ' MB';
    else return (bytes / (1024 * 1024 * 1024)).toFixed(0) + ' GB';
}

</script>

<style></style>