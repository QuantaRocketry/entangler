use defmt::Format;
use embassy_sync::{blocking_mutex::raw::CriticalSectionRawMutex, mutex::Mutex};

pub static SYSTEM_STATE: Mutex<CriticalSectionRawMutex, SystemState> = Mutex::new(SystemState {
    lora_state: LoRaState {},
});

#[derive(Format)]
pub struct SystemState {
    pub lora_state: LoRaState,
}

#[derive(Debug, Clone, PartialEq, Format, Copy)]
pub struct LoRaState {}
