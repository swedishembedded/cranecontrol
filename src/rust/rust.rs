#![crate_type="staticlib"]

#![no_std]
use core::panic::PanicInfo;

pub enum CVoid {}

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
	loop {

	}
}

extern "C" {
	pub fn printk(fmt: *const i8, ...) -> i32;
	pub fn snprintf(s: *const i8, num: i32, fmt: *const i8, ...) -> i32;
	pub fn _console_printf(con: *const CVoid, fmt: *const i8, ...) -> i32;
}

#[no_mangle]
pub extern "C" fn rust_function(con: *mut CVoid) {
    let buffer: [i8; 32] = [42; 32];
	unsafe {
        snprintf(buffer.as_ptr() as *const i8, buffer.len() as i32, "num: %d\n\0".as_ptr() as *const i8, 12);
        printk(buffer.as_ptr() as *const i8);
        _console_printf(con, "Hello from RUST 1\n\0".as_ptr() as *const i8);
        printk("Hello From RUST!\n\0".as_ptr() as *const i8);
	}
}


