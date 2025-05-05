build *args:
    @if [[ -n "{{args}}" ]]; then \
        west build ./app -b {{args}} -p auto -DCMAKE_EXPORT_COMPILE_COMMANDS=1; \
    else \
        west build ./app -DCMAKE_EXPORT_COMPILE_COMMANDS=1; \
    fi

upload:
    #!/usr/bin/env bash
    picotool load -v -x build/zephyr/zephyr.uf2 -f

upload-delay:
    sleep 2

connect:
    #!/usr/bin/env bash
    VID="2e8a"
    DEV=$(ls /dev/ttyACM* 2>/dev/null | while read d; do
        if lsusb | grep -q "$VID"; then
            echo "$d";  break;
        fi
    done)
    minicom -D "$DEV" || echo "Device not found"

run: build upload upload-delay connect
