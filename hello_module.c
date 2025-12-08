/// Это будет код модуля, который можно будет собрать и прилинковать к
/// кернелу в runtime, используя insmod (rmmod для того, чтобы отключить)

/// @b Char-module - для потоковых устройств, а-ля запись в файл. Например,
/// консоль или serial-port
/// @b Block-module - хостит файловую систему, сильнее отличаются в ядре от
/// char, но могут быть использованы так же
/// @b Netwrok-module - устройства, обменивающиеся информацией с другими
/// хостами. Обрабатывает пакеты

/// USB-драйвера являются отдельными модулями и могут отображаться в системе как
/// любой из трех видов. Модули пишутся с учетом многопоточности, т.к. никогда не
/// могут быть уверены в том, что несколько строк кода будут выполнены без прерываний

/// @b Небольшой-стек у приложений kernel (напр. одна страница 4K), поэтому
/// большие структуры в heap. Не желательно использовать FPU, т.к. его состояние
/// придется сохранять при переключении контекста.

#include <linux/init.h>
#include <linux/module.h>

#define DEBUG KERN_DEBUG "__wusb__ "

MODULE_LICENSE("Dual BSD/GPL");

/// @brief Сообщение из Kernel, которое показывается через journalctl
/// @param data
static void debug_print(const char* data)
{
    printk(DEBUG "%s\n", data);
}

/// @b file - отличается от user FILE, содержится только в kernel. Передается
/// user при запросе на операции с устройствами-файлами. Является представлением
/// открытого файла
static ssize_t read_device(struct file* file, char* path, size_t, loff_t*) {
  return 0;
}

/// @b __init используется, чтобы подсказать kernel, что
/// эту функцию можно удалить после запуска, т.к. она больше
/// не будет нигде использоваться
static int __init hello_init(void)
{
    debug_print("Hello");
    
    /// @b Device-numbers - мажор-минор цифры, которые нужны, чтобы 
    /// идентифицировать устройство, отличать файл в /dev. Используется
    /// динамическая аллокация, поскольку мажор отвечает за вид устройства,
    /// номер для которого заранее неизвестен.
    dev_t dev;
    int result = alloc_chrdev_region(&dev, 0, 1, "wirelessusb");
    if (result < 0) {
        printk(DEBUG "cannot get major version\n");
        return result;
    } else
        printk(DEBUG "device numbers: %u, %u\n", MAJOR(dev), MINOR(dev));
    unregister_chrdev_region(dev, 1);

    /// @b File_operations - структура, переопределяющая использование файлового API
    /// для устройства. Вызовы вроде open, close, read, write перенаправляются сюда из user
    struct file_operations fop;
    fop.read = read_device;

    return 0;
}

/// @b __exit помещает функцию в специальное место ELF, чтобы
/// оптимально вызывать ее ТОЛЬКО во время деинициализации.
static void __exit hello_exit(void)
{
    debug_print("Goodbye");
}

/// @b Инициализация - подготовка модуля при подключении к kernel
module_init(hello_init);
/// @b Деинициализация - код для отключения от kernel
module_exit(hello_exit);