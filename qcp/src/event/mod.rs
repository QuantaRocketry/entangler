mod id_01_heartbeat;
pub use id_01_heartbeat::Heartbeat;

pub enum Event {
    Heartbeat(Heartbeat),
}