use std::ffi::c_void;

pub use crate::buffer;
use crate::Vertex;
pub use gl::*;

use self::buffer::{BufferLayout, IndexBuffer, VertexBuffer};
pub struct OpenGLVertexBuffer {
    id: u32,
    size: isize,
    layout: buffer::BufferLayout,
}

impl OpenGLVertexBuffer {
    pub fn new(layout: BufferLayout) -> OpenGLVertexBuffer {
        let id = 0;
        unsafe {
            CreateBuffers(1, id as *mut u32);
        }
        OpenGLVertexBuffer {
            id,
            layout,
            size: 0,
        }
    }
}

impl VertexBuffer for OpenGLVertexBuffer {
    fn bind(&self) {
        unsafe {
            BindBuffer(ARRAY_BUFFER, self.id);
        }
    }

    fn unbind(&self) {
        unsafe {
            BindBuffer(ARRAY_BUFFER, 0);
        }
    }

    fn set_data(&mut self, buffer: &Vec<Vertex>, size: isize) {
        unsafe { NamedBufferStorage(self.id, size, buffer.as_ptr() as *const c_void, 0) }
        self.size = size;
    }
}

pub struct OpenGLIndexBuffer {
    id: u32,
    size: isize,
}

impl OpenGLIndexBuffer {
    pub fn new() -> OpenGLIndexBuffer {
        let id: u32 = 0;
        unsafe {
            CreateBuffers(1, id as *mut u32);
        }
        OpenGLIndexBuffer { id, size: 0 }
    }
}

impl IndexBuffer for OpenGLIndexBuffer {
    fn bind(&self) {
        unsafe {
            BindBuffer(ELEMENT_ARRAY_BUFFER, self.id);
        }
    }

    fn unbind(&self) {
        unsafe {
            BindBuffer(ELEMENT_ARRAY_BUFFER, 0);
        }
    }

    fn set_data(&mut self, buffer: &Vec<Vertex>, size: isize) {
        unsafe { NamedBufferStorage(self.id, size, buffer.as_ptr() as *const c_void, 0) }
        self.size = size;
    }
}
