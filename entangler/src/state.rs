use defmt::Format;
use embassy_sync::{blocking_mutex::raw::CriticalSectionRawMutex, mutex::Mutex};

pub static _SYSTEM_STATE: Mutex<CriticalSectionRawMutex, SystemState> = Mutex::new(SystemState {
    lora_state: LoRaState {},
    settings: SystemSettings {},
});

#[derive(Format, Clone, Default)]
pub struct SystemState {
    pub lora_state: LoRaState,
    pub settings: SystemSettings,
    // pub error: SystemError,
}

#[derive(Debug, Clone, PartialEq, Format, Copy, Default)]
pub struct SystemSettings {}

#[derive(Debug, Clone, PartialEq, Format, Copy, Default)]
pub struct LoRaState {}
