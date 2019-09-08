# LMutex
LMutex - синхронный/асинхронный мьютекс для Qt-приложений.
## Возможности:
+ В синхронном режиме - блокирование потока до момента доступности ресурса (аналог `QMutex`)
+ В ассинхронном режиме - не блокирование потока при недоступности ресурса и выполнение определенного метода объекта после доступности ресурса
+ Разграничение операций на чтение и запись по аналогии с `QReadWriteLock`
+ Ограничение количества одновременного читающих ресурс потоков (readers) и одновременного изменяющих ресурс потоков (writers)
+ Установка приоритета на постановку в очередь на доступ ресурсу
+ Определение своего алгоритма формирования очереди выполнения запросов на доступ ресурсу
## Примеры:
+ Синхронный режим
```
lmutex.waitForRead();
// working with the resource
lmutex.release();
```
+ Асинхронный режим
```
void Worker::acquereForRead() {
	lmutex.acquereForRead(this, "onReadyRead");
	// do something else
}

void Worker::onReadyRead() { // declare method as a slot
	// working with the resource
	lmutex.release();
}
```
или с помощью lambda-функции
```
lmutex.acquereForRead(this, [=](){
	// working with the resource
	lmutex.release();
}