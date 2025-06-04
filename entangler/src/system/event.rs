use embassy_sync::{blocking_mutex::raw::CriticalSectionRawMutex, channel::Channel};

pub static EVENT_CHANNEL: Channel<CriticalSectionRawMutex, Event, 10> = Channel::new();

pub async fn send_event(event: Event) {
    EVENT_CHANNEL.sender().send(event).await;
}

pub async fn wait() -> Event {
    EVENT_CHANNEL.receiver().receive().await
}

#[derive(Debug, Clone)]
pub enum Event {}

