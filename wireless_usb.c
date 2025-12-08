#include <linux/hid.h>
#include <linux/init.h>
#include <linux/module.h>

#define DEBUG KERN_DEBUG "__wusb__ "

MODULE_LICENSE("Dual BSD/GPL");

static const struct hid_device_id hid_example_table[] = {
    { HID_USB_DEVICE(0x2563, 0x0526) },
    {}
};
MODULE_DEVICE_TABLE(hid, hid_example_table);

struct my_hid_driver {
    struct hid_device* device;
};
static struct my_hid_driver* drv_data;

/// @b Probe функция для реагирование на присоединение устройства из таблицы,
/// обязательна для hid-устройства, т.к. для его обработки нужно обязательно:
/// - hid_parse - 
/// - hid_hw_start - включает HW-буферы, нужно вызывать после hid_parse
/// - hid_hw_open - говорит HW отправлять события, без него ничего не будет 
///                 приходить в event
static int hid_example_probe(struct hid_device* hdev, const struct hid_device_id* id)
{
    int ret;

    printk(DEBUG "HID Example: Device connected!\n");
    printk(DEBUG "  Vendor: 0x%04x\n", id->vendor);
    printk(DEBUG "  Product: 0x%04x\n", id->product);
    printk(DEBUG "  Bus: %s\n", hdev->bus == BUS_USB ? "USB" : "Other");
    printk(DEBUG "  Name: %s\n", hdev->name);

    ret = hid_parse(hdev);
    if (ret) {
        printk(DEBUG "Failed to parse HID descriptor\n");
        return ret;
    }

    ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
    if (ret) {
        hid_hw_stop(hdev);
        printk(DEBUG "Failed to start HID hardware\n");
        return ret;
    }

    hid_set_drvdata(hdev, (void*)id);
    printk(DEBUG "HID Example: Driver loaded successfully\n");

    /// @b devm_kzalloc - device-managed kernel zeroed allocation,
    /// выделение глобального объекта для драйвера, что будет освобожден
    /// после отключения устройства
    drv_data = devm_kzalloc(&hdev->dev, sizeof(*drv_data), GFP_KERNEL);
    hid_set_drvdata(hdev, drv_data);
    drv_data->device = hdev;

    ret = hid_hw_open(hdev);
    if (ret) {
        printk(DEBUG " device fail to open\n");
        return ret;
    } else {
        printk(DEBUG " device opened\n");
    }
    return 0;
}

// ====================== REMOVE функция ======================
static void hid_example_remove(struct hid_device* hdev)
{
    const struct hid_device_id* id = hid_get_drvdata(hdev);
    if (id) {
        printk(DEBUG "HID Example: Device removed (%04x:%04x)\n",
            id->vendor, id->product);
    } else {
        printk(DEBUG "HID Example: Device removed\n");
    }
    hid_hw_stop(hdev);
    devm_kfree(&hdev->dev, drv_data);
}

static const char* to_string(__u16 code)
{
    switch (code) {
        case BTN_A:
            return "A";
        case BTN_B:
            return "B";
        case BTN_X:
            return "X";
        case BTN_Y:
            return "Y";
        case BTN_TL:
            return "LB";
        case BTN_TR:
            return "RB";
        case BTN_TL2:
            return "LT";
        case BTN_TR2:
            return "RT";
        case BTN_START:
            return "start";
        case BTN_SELECT:
            return "select";
        case BTN_THUMBL:
            return "LS";
        case BTN_THUMBR:
            return "RS";
        default:
            break;
    }
    return "unk";
}

static const char* to_string_abs(__u16 code)
{
    switch (code) {
        case ABS_GAS:
            return "LT";
        case ABS_BRAKE:
            return "RT";
        case ABS_HAT0X:
            return "Cross";
        case ABS_X:
            return "LS_x";
        case ABS_Y:
            return "LS_y";
        case ABS_Z:
            return "RS_x";
        case ABS_RZ:
            return "RS_y";
        default:
            break;
    }
    return "unk";
}

static int event_occured(struct hid_device* hdev, struct hid_field* field,
    struct hid_usage* usage, __s32 value)
{
    switch (usage->type) {
        case EV_KEY:
            if (value)
                printk(DEBUG "tap %s\n", to_string(usage->code));
            break;
        case EV_ABS:
            printk(DEBUG " range %s: %i\n", to_string_abs(usage->code), value - 128);
            break;
        case EV_SYN:
            /// @b Синхронизация нужна потому, что устройство шлет 
            /// потоком сообщения о всех позициях триггеров и состояниях кнопок,
            /// это сообщение приходит последним из всего потока
            break;
        default:
            printk(DEBUG "HID: [%04x:%04x] type=%d code=%d value=%d\n",
                usage->hid >> 16, usage->hid & 0xffff,
                usage->type, usage->code, value);
    }
    return 0;
}

// ====================== СТРУКТУРА ДРАЙВЕРА ======================
static struct hid_driver hid_example_driver = {
    .name = "wireless_usb",
    .id_table = hid_example_table,
    .probe = hid_example_probe,
    .remove = hid_example_remove,
    .event = event_occured,
};

// ====================== МОДУЛЬ ======================
module_hid_driver(hid_example_driver);