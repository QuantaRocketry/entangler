use super::MessageTrait;

use zerocopy::*;

#[derive(Debug, Eq, PartialEq, Copy, Clone, FromBytes, IntoBytes, KnownLayout, Immutable)]
#[repr(C)]
pub struct Request {
    pub messages: u32,
}

impl MessageTrait for Request {
    const PACKET_ID: u8 = 2;
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn direct() {
        let output: &[u8] = Request { messages: 13298326 }.as_bytes();

        assert_eq!(&[150, 234, 202, 0], output);

        let out: &Request = Request::ref_from_bytes(output).unwrap();
        assert_eq!(out, &Request { messages: 13298326 });
    }
}
