use crate::{error::ProtocolError, message::Message, message::MessageType};
mod version;
pub use version::ProtocolVersion;

use core::mem::size_of;
use zerocopy::*;

pub const HEADER_SIZE: u8 = 6;

#[derive(Debug, PartialEq, Eq, Clone, Copy, Immutable, KnownLayout, IntoBytes)]
#[repr(C)]
pub struct Header {
    pub lrc: u8,
    pub version: ProtocolVersion,
    pub length: u8,
    pub message_type: MessageType,
    pub crc16: u16,
}

impl Header {
    pub fn deserialize(buf: &[u8]) -> Result<Self, ProtocolError> {
        if buf.len() < size_of::<Header>() {
            return Err(ProtocolError::BufferTooSmall);
        }

        let version = ProtocolVersion::try_ref_from_bytes(&buf[1..])
            .map_err(|_| ProtocolError::UnknownVersion(buf[1]))?
            .clone();

        let message_type = MessageType::try_ref_from_bytes(&buf[3..])
            .map_err(|_| ProtocolError::UnknownMessageType(buf[3]))?
            .clone();

        let header = Header {
            lrc: buf[0],
            version,
            length: buf[2],
            message_type,
            crc16: u16::ref_from_bytes(&buf[4..])
                .map_err(|_| ProtocolError::InvalidCRC16)?
                .clone(),
        };

        Ok(header)
    }
}

impl From<&Message> for Header {
    fn from(msg: &Message) -> Self {
        let todo = true; // TODO calculate all the checks
        Header {
            lrc: 0,
            version: ProtocolVersion::CURRENT_VERSION,
            length: 0,
            message_type: MessageType::from(msg),
            crc16: 0,
        }
    }
}
