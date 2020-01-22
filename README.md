# LReadWriteLock
LReadWriteLock - аналог QReadWriteLock, но с поддержкой асинхронного доступа.
## Возможности:
+ Синхронный досутп - блокирование потока до момента доступности ресурса по аналогии с `QMutex`
+ Ассинхронный доступ - выполнение определенного метода объекта или лямбда-фунции после доступности ресурса потоку без его блокировки
+ Разграничение операций на чтение и запись по аналогии с `QReadWriteLock`
+ Ограничение количества одновременного читающих ресурс потоков (readers) и одновременного изменяющих ресурс потоков (writers)
+ Задание приоритета выполнения задач
+ Задание политики учета пищущих задач находящихся в очереди читающими задачами
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