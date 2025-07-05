use core::fmt;

/// Errors that can occur during protocol serialization or deserialization.
#[derive(Debug, PartialEq, Eq)]
pub enum ProtocolError {
    /// The provided buffer is too small to hold the serialized message.
    BufferTooSmall,
    /// The input bytes are too short to form a complete message.
    InputTooShort,
    /// An unknown `version` byte was encountered.
    UnknownVersion(u8),
    /// An unknown `MessageType` byte was encountered.
    UnknownMessageType(u8),
    /// The message payload length is invalid or exceeds limits.
    InvalidPayloadLength,
    /// Error converting bytes to a float (should not happen with correct data).
    InvalidFloatConversion,
    /// Error with CRC16.
    InvalidCRC16,
}

impl fmt::Display for ProtocolError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            ProtocolError::BufferTooSmall => {
                write!(f, "Buffer provided is too small to serialize message")
            }
            ProtocolError::InputTooShort => {
                write!(f, "Input bytes are too short to deserialize message")
            }
            ProtocolError::UnknownVersion(byte) => {
                write!(f, "Unknown protocol version: {}", byte)
            }
            ProtocolError::UnknownMessageType(msg_type) => {
                write!(f, "Unknown message type: {}", msg_type)
            }
            ProtocolError::InvalidPayloadLength => {
                write!(f, "Invalid payload length specified in message")
            }
            ProtocolError::InvalidFloatConversion => write!(f, "Failed to convert bytes to float"),
            ProtocolError::InvalidCRC16 => write!(f, "Invalid CRC16"),
        }
    }
}
