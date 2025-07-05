mod id_01_heartbeat;
pub use id_01_heartbeat::Heartbeat;

mod id_02_request;
pub use id_02_request::Request;

mod id_03_gnss;
pub use id_03_gnss::GNSS;
use zerocopy::FromBytes;
use zerocopy::Immutable;
use zerocopy::IntoBytes;
use zerocopy::KnownLayout;
use zerocopy::TryFromBytes;

use crate::error::ProtocolError;
use crate::header::Header;

pub trait MessageTrait: FromBytes + Immutable + IntoBytes {
    const PACKET_ID: u8;
}

#[derive(Debug, PartialEq, Eq, Clone)]
pub enum Message {
    Heartbeat(Heartbeat),
    Request(Request),
    GNSS(GNSS),
}

#[derive(Debug, PartialEq, Eq, Clone, Copy, TryFromBytes, Immutable, IntoBytes, KnownLayout)]
#[repr(u8)]
pub enum MessageType {
    Unknown = 0x00,
    Heartbeat = Heartbeat::PACKET_ID,
    Request = Request::PACKET_ID,
    GNSS = GNSS::PACKET_ID,
}

impl From<&Message> for MessageType {
    fn from(value: &Message) -> Self {
        match value {
            Message::Heartbeat(_) => MessageType::Heartbeat,
            Message::Request(_) => MessageType::Request,
            Message::GNSS(_) => MessageType::GNSS,
        }
    }
}

impl Message {
    pub fn serialize(self: &Self, buf: &mut [u8]) -> Result<(), ProtocolError> {
        if buf.len() < size_of::<Header>() {
            return Err(ProtocolError::BufferTooSmall);
        }

        // TODO add header to buf
        let header = Header::from(self);

        match self {
            Message::Heartbeat(msg) => msg.write_to(buf).map_err(|_| ProtocolError::BufferTooSmall),
            Message::Request(msg) => msg.write_to(buf).map_err(|_| ProtocolError::BufferTooSmall),
            Message::GNSS(msg) => msg.write_to(buf).map_err(|_| ProtocolError::BufferTooSmall),
        }
    }

    pub fn deserialize(buf: &[u8]) -> Result<Self, ProtocolError> {
        let header = Header::deserialize(buf)?;

        Ok(match header.message_type {
            MessageType::Unknown => todo!(),
            MessageType::Heartbeat => Message::Heartbeat(
                Heartbeat::ref_from_bytes(buf)
                    .map_err(|_| ProtocolError::InputTooShort)?
                    .clone(),
            ),
            MessageType::Request => Message::Request(
                Request::ref_from_bytes(buf)
                    .map_err(|_| ProtocolError::InputTooShort)?
                    .clone(),
            ),
            MessageType::GNSS => todo!(),
        })
    }
}
