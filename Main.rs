use std::io;
use winit::event_loop::EventLoop;

fn main() {
    println!("Hello, world!");

    let mut cmdgui = String::new();
    io::stdin()
        .read_line(&mut cmdgui)
        .expect("Failed to read line");

    if cmdgui.trim() == "gui" {
        println!("Starting Athena backend GUI");    
    } else {
        println!("Starting Athena backend CMD");
    }
}