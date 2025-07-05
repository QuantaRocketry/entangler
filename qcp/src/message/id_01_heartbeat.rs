use super::MessageTrait;

use zerocopy::*;

#[derive(Debug, Eq, PartialEq, Copy, Clone, FromBytes, IntoBytes, KnownLayout, Immutable)]
#[repr(C)]
pub struct Heartbeat {
    pub uptime: u32,
}

impl MessageTrait for Heartbeat {
    const PACKET_ID: u8 = 1;
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::Message;
    use crate::PACKET_SIZE_MAX;

    #[test]
    fn direct() {
        let output: &[u8] = Heartbeat { uptime: 13298326 }.as_bytes();

        assert_eq!(&[150, 234, 202, 0], output);

        let out: &Heartbeat = Heartbeat::ref_from_bytes(output).unwrap();
        assert_eq!(out, &Heartbeat { uptime: 13298326 });
    }

    #[test]
    fn indirect() {
        let mut buf: [u8; PACKET_SIZE_MAX] = [0; PACKET_SIZE_MAX];

        Message::Heartbeat(Heartbeat { uptime: 13298326 })
            .serialize(&mut buf[0..])
            .unwrap();

        assert_eq!(&[150, 234, 202, 0], &buf[..4]);

        let out: &Heartbeat = Heartbeat::ref_from_bytes(&buf[..4]).unwrap();
        assert_eq!(out, &Heartbeat { uptime: 13298326 });
    }
}
