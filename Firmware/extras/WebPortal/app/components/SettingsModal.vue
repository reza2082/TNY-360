<template>
    <div class="flex flex-col space-y-4">
        <div class="space-y-2">
            <h2 class="text-xl font-semibold"> Wi-Fi </h2>
            <div class="flex space-x-4 justify-start items-end pl-4">
                <UFormField label="SSID">
                    <UInput v-model="ssid" type="text" />
                </UFormField>
                <UFormField label="Password">
                    <UInput v-model="password" type="password" />
                </UFormField>
                <UButton class="w-fit h-fit" label="Connect" :loading="wifiConnectBtnLoading" @click="onConnect" />
            </div>
        </div>
    </div>
</template>

<script lang="ts" setup>
const remote = useRemote();

const ssid = ref('');
const password = ref('');

const wifiConnectBtnLoading = ref(false);
async function onConnect() {
    wifiConnectBtnLoading.value = true;
    try {
        await remote.connectToAP(ssid.value, password.value);
    } catch (err) {

    }
    wifiConnectBtnLoading.value = false;
}

</script>