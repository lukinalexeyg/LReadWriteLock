# LReadWriteLock
LReadWriteLock - синхронно-асинхронный блокировщик-менеджер предоставления ресурса (критической секции). Аналог QReadWriteLock, но с одновременной поддержкой как синхронного так и асинхронного доступа.
## Возможности:
+ Синхронный доступ - блокирование потока до момента доступности ресурса по аналогии с `QMutex`
+ Асинхронный доступ - выполнение определенного метода объекта или лямбда-функции после доступности ресурса потоку без его блокировки
+ Разграничение операций на чтение и запись по аналогии с `QReadWriteLock`
+ Ограничение количества одновременного читающих ресурс потоков (readers) и одновременного изменяющих ресурс потоков (writers)
+ Задание приоритета выполнения задач
+ Задание политики учета пишущих задач находящихся в очереди читающими задачами
+ Удаление похожих задач из очереди по различным критериям
## Примеры:
+ Синхронный запрос
```c
lmutex.waitForRead();
// working with the resource
lmutex.release();
```
+ Асинхронный запрос
```c
void Worker::acquereForRead() {
    lmutex.acquereForRead(this, "onReadyRead");
    // do something else
}

void Worker::onReadyRead() { // declare method as a slot
    // working with the resource
    lmutex.release();
}
```
или с помощью лямбда-функции
```c
lmutex.acquereForRead(this, [=](){
    // working with the resource
    lmutex.release();
}
