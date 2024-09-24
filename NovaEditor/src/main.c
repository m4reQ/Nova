#include <Nova/core/application.h>

static bool EditorOnLoad(void)
{
    return true;
}

static void EditorOnClose(void)
{
}

static void EditorOnUpdate(double frametime)
{
}

int main(int argc, char **argv)
{
    const NvApplication app = {
        .onLoad = EditorOnLoad,
        .onClose = EditorOnClose,
        .onUpdate = EditorOnUpdate,
        .name = "NovaEditor",
        .windowSettings = {
            .width = 720,
            .height = 640,
            .title = L"Nova Editor",
            .windowFlags = NV_WND_FLAGS_DEFAULT,
        },
    };

    return NvApplicationRun(&app);
}
