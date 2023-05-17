#include "filesystem.h"

#include "core/logger.h"
#include "core/sfmemory.h"

// TODO: make it actually platform specific
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

b8 filesystem_exists(const char *path) {
		struct stat buffer;
		return stat(path, &buffer) == 0;
}

b8 filesystem_open(const char *path, file_modes mode, b8 binary,
				   file_handle *out_handle) {
		out_handle->is_valid = FALSE;
		out_handle->handle = SF_NULL;
		const char *mode_str;

		if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) != 0) {
				mode_str = binary ? "w+b" : "w+";
		} else if ((mode & FILE_MODE_READ) != 0) {
				mode_str = binary ? "rb" : "r";
		} else if ((mode & FILE_MODE_WRITE) != 0) {
				mode_str = binary ? "wb" : "w";
		} else {
				SF_ERROR("Invalid mode passed when attempting to read file: %s",
						 path);
				return FALSE;
		}

		FILE *file = fopen(path, mode_str);
		if (!file) {
				SF_ERROR("Error opening file: %s", path);
				return FALSE;
		}

		out_handle->is_valid = TRUE;
		out_handle->handle = file;
		return TRUE;
}

void filesystem_close(file_handle *handle) {
		if (!handle->handle || !handle->is_valid) {
				return;
		}
		fclose((FILE *)handle->handle);
		handle->handle = SF_NULL;
		handle->is_valid = FALSE;
}

b8 filesystem_read_line(file_handle *handle, char **line_buf) {
		if (!handle->handle || !handle->is_valid) {
				return FALSE;
		}
		// Assuming it'll fit, if not -> extend
		char buffer[16000];
		if (fgets(buffer, 16000, (FILE *)handle->handle) != 0) {
				u64 len = strlen(buffer);
				*line_buf =
					sfalloc((sizeof(char) * len) + 1, MEMORY_TAG_STRING);
				strcpy(*line_buf, buffer);
				return TRUE;
		}
		return FALSE;
}

b8 filesystem_write_line(file_handle *handle, const char *text) {
		if (!handle->handle || !handle->is_valid) {
				return FALSE;
		}
		i32 result = fputs(text, (FILE *)handle->handle);
		if (result != EOF) {
				result = fputc('\n', (FILE *)handle->handle);
		}
		fflush((FILE *)handle->handle);
		return result != EOF;
}

b8 filesystem_read(file_handle *handle, u64 data_size, void *out_data,
				   u64 *out_bytes_read) {
		if (!handle->handle || !handle->is_valid || !out_bytes_read) {
				return FALSE;
		}
		*out_bytes_read = fread(out_data, 1, data_size, (FILE *)handle->handle);
		if (*out_bytes_read != data_size) {
				return FALSE;
		}
		return TRUE;
}

b8 filesystem_read_all_bytes(file_handle *handle, u8 **out_bytes,
							 u64 *out_bytes_read) {
		if (!handle->handle || !handle->is_valid) {
				return FALSE;
		}
		fseek((FILE *)handle->handle, 0, SEEK_END);
		u64 size = ftell((FILE *)handle->handle);
		rewind((FILE *)handle->handle);

		*out_bytes = sfalloc(sizeof(u8) * size, MEMORY_TAG_STRING);
		*out_bytes_read = fread(*out_bytes, 1, size, (FILE *)handle->handle);
		if (*out_bytes_read != size) {
				return FALSE;
		}
		return TRUE;
}

b8 filesystem_write(file_handle *handle, u64 data_size, const void *data,
					u64 *out_bytes_written) {
		if (!handle->handle || !handle->is_valid) {
				return FALSE;
		}
		*out_bytes_written = fwrite(data, 1, data_size, (FILE *)handle->handle);
		if (*out_bytes_written != data_size) {
				return FALSE;
		}
		fflush((FILE *)handle->handle);
		return TRUE;
}