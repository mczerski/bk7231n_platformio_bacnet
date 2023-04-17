#include "bacnet/awf.h"
#include "bacnet/basic/object/bacfile.h"
#include "Update.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
bool bacfile_write_stream_data(BACNET_ATOMIC_WRITE_FILE_DATA *data);
#ifdef __cplusplus
}
#endif /* __cplusplus */

void onProgress(size_t bytesWritten, size_t bytesTotal) {
    printf("Bytes written: %d, bytes total: %d\n", bytesWritten, bytesTotal);
}

void update_init() {
    bacfile_create(1);
    bacfile_pathname_set(1, "firmware.bin");
    Update.onProgress(onProgress);
}

bool bacfile_write_stream_data(BACNET_ATOMIC_WRITE_FILE_DATA *data)
{
    if (data->type.stream.fileStartPosition == 0) {
        if (Update.isRunning()) {
            printf("Update already running, aborting...\n");
            Update.abort();
            Update.clearError();
        }
        if (not Update.begin()) {
            printf("Failed to start update\n");
            return false;
        }
    }
    Update.write(octetstring_value(&data->fileData[0]), octetstring_length(&data->fileData[0]));
    if (Update.hasError()) {
        printf(Update.errorString());
        return false;
    }
    if (Update.remaining() == 0) {
        if (not Update.end()) {
            printf("Failed to end update\n");
            return false;
        }
    }
    return true;
}
