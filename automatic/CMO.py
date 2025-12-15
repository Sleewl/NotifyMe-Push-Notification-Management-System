import heapq
import random

# --- Константы ---
T_ALPHA = 1.643  # t_alpha для доверительной вероятности 0.9
DELTA = 0.1      # Относительная точность 10%
MAX_ITERATIONS = 10 # Защита от бесконечного цикла

# --- Структура для хранения информации о заявке (уведомлении) ---
class Request:
    def __init__(self, id, source_id, generation_time):
        self.id = id
        self.source_id = source_id
        self.generation_time = generation_time
        self.start_service_time = -1.0
        self.end_service_time = -1.0
        self.was_rejected = False
        self.status = "NEW" # Статус заявки: NEW, IN_BUFFER, PROCESSING, COMPLETED, REJECTED

    def __repr__(self):
        return f"Request(id={self.id}, src={self.source_id}, gen_time={self.generation_time}, status={self.status})"

# --- Структура для события ---
class Event:
    def __init__(self, time, event_type, data=None):
        self.time = time
        self.type = event_type  # 'GENERATE', 'FREE_CHANNEL'
        self.data = data        # Дополнительные данные (например, Request или channel_id)

    def __lt__(self, other):
        # heapq нужен min-heap, поэтому __lt__ для сортировки по времени
        return self.time < other.time

    def __repr__(self):
        return f"Event(time={self.time}, type={self.type}, data={self.data})"

# --- Класс Клиента (ИБ, ИЗ1) ---
class Client:
    def __init__(self, avg_interval, num_clients):
        self.avg_interval = avg_interval
        self.lambda_param = 1.0 / avg_interval
        self.num_clients = num_clients
        self.next_id_per_client = {i: 1 for i in range(1, num_clients + 1)} # Счетчик ID для каждого клиента

    def generate_request(self, current_time, client_id):
        interval = random.expovariate(self.lambda_param)
        next_gen_time = current_time + interval
        req = Request(self.next_id_per_client[client_id], client_id, next_gen_time)
        self.next_id_per_client[client_id] += 1
        return req

# --- Класс Специалиста (П32) ---
class Specialist:
    def __init__(self, specialist_id):
        self.id = specialist_id
        self.is_busy = False
        self.busy_until_time = 0.0
        self.total_busy_time = 0.0

    def is_free_at_time(self, time):
        return not self.is_busy or self.busy_until_time <= time

    def start_service(self, start_time, service_duration):
        if self.is_busy and self.busy_until_time > start_time:
            start_time = self.busy_until_time
        self.is_busy = True
        self.busy_until_time = start_time + service_duration
        self.total_busy_time += service_duration

    def free_specialist(self):
        self.is_busy = False
        self.busy_until_time = 0.0

# --- Класс Буфера (Д1ОЗ1, Д1ОО4) ---
class Buffer:
    def __init__(self, size):
        self.size = size
        self.slots = [None] * size
        self.count = 0
        self.pointer = 0 # Указатель для Д1ОЗ1 и Д2БЗ

    def is_full(self):
        return self.count == self.size

    def is_empty(self):
        return self.count == 0

    # Добавление заявки (Д1ОЗ1 - по кольцу)
    # Возвращает: (True, evicted_request) или (True, None)
    # ВСЕГДА добавляет новую заявку, если буфер не полон. Если полон - вытесняет и добавляет.
    def add_request(self, req):
        evicted_req = None
        if self.is_full():
            # Вытеснение последней поступившей (Д1ОО4)
            evicted_req = self.remove_last_request()
            if evicted_req is not None:
                # Заявка, которая была вытеснена, получает отказ
                evicted_req.was_rejected = True
                evicted_req.status = "REJECTED"
                # Теперь добавляем новую заявку
                # Ищем свободное место, начиная с указателя
                for i in range(self.size):
                    index = (self.pointer + i) % self.size
                    if self.slots[index] is None:
                        self.slots[index] = req
                        self.count += 1
                        self.pointer = (index + 1) % self.size # Обновляем указатель
                        req.status = "IN_BUFFER"
                        return True, evicted_req # Успешно добавлено с вытеснением
                # Не должно сработать, если is_full() работал корректно
                return False, None
            else:
                # Не удалось вытеснить, не должно сработать
                return False, None
        else:
            # Буфер не полон, просто добавляем
            # Ищем свободное место, начиная с указателя
            for i in range(self.size):
                index = (self.pointer + i) % self.size
                if self.slots[index] is None:
                    self.slots[index] = req
                    self.count += 1
                    self.pointer = (index + 1) % self.size # Обновляем указатель
                    req.status = "IN_BUFFER"
                    return True, None # Успешно добавлено без вытеснения
            return False, None # Не должно сработать при is_empty()

    # Вытеснение последней поступившей (Д1ОО4) - ВОЗВРАЩАЕТ вытесненную заявку
    def remove_last_request(self):
        if self.is_empty():
            return None
        latest_time = -1.0
        latest_pos = -1
        for i in range(self.size):
            if self.slots[i] is not None and self.slots[i].generation_time > latest_time:
                latest_time = self.slots[i].generation_time
                latest_pos = i
        if latest_pos != -1:
            evicted_req = self.slots[latest_pos]
            self.slots[latest_pos] = None
            self.count -= 1
            # Не меняем pointer, так как он указывает на следующее *свободное* место
            return evicted_req
        return None

    # Выбор заявки из буфера (Д2БЗ - по кольцу)
    def get_next_by_pointer(self):
        if self.is_empty():
            return None
        for i in range(self.size):
            index = (self.pointer + i) % self.size
            if self.slots[index] is not None:
                req = self.slots[index]
                self.slots[index] = None # Удаляем из буфера
                self.count -= 1
                self.pointer = (index + 1) % self.size # Обновляем указатель
                return req
        return None # Не должно сработать при is_empty()

    def clear(self):
        self.slots = [None] * self.size
        self.count = 0
        self.pointer = 0

# --- Класс Размещения (PlacementDispatcher) ---
class PlacementDispatcher:
    def __init__(self, specialists, buffer, service_min=0.8, service_max=1.2):
        self.specialists = specialists
        self.buffer = buffer
        self.service_min = service_min
        self.service_max = service_max

    # Планирует обслуживание на специалисте
    def dispatch_to_specialist(self, req, current_time):
        # Д2П1 - приоритет по номеру специалиста (П1, П2, ...)
        for specialist in self.specialists:
            if specialist.is_free_at_time(current_time):
                # П32 - равномерное время обслуживания
                service_duration = random.uniform(self.service_min, self.service_max)
                req.start_service_time = current_time
                req.end_service_time = current_time + service_duration
                specialist.start_service(current_time, service_duration)
                req.status = "PROCESSING"
                return True # Назначено
        return False # Все специалисты заняты

# --- Класс Выбора (SelectionDispatcher) ---
class SelectionDispatcher:
    def __init__(self, placement_dispatcher, buffer):
        self.placement_dispatcher = placement_dispatcher
        self.buffer = buffer

    # Выбирает заявку из буфера и назначает на специалиста
    def select_and_dispatch(self, current_time):
        next_req = self.buffer.get_next_by_pointer()
        if next_req:
            assigned = self.placement_dispatcher.dispatch_to_specialist(next_req, current_time)
            if assigned:
                return True, next_req # Успешно назначено
            else:
                # Это маловероятно, но на всякий случай
                return False, next_req # Не назначено
        return False, None # Буфер пуст

# --- Класс Автоматического Режима ---
class AutoModeSimulator:
    def __init__(self, client, buffer, specialists, placement_dispatcher, fixed_n=5000, num_clients=10):
        self.client = client
        self.buffer = buffer
        self.specialists = specialists
        self.placement_dispatcher = placement_dispatcher
        self.N = fixed_n
        self.num_clients = num_clients

    def run_simulation(self, num_requests):
        # Сброс системы
        self.buffer.clear()
        for sp in self.specialists:
            sp.is_busy = False
            sp.busy_until_time = 0.0
            sp.total_busy_time = 0.0

        # --- Инициализация календаря событий ---
        event_calendar = []

        # --- Генерация N заявок и добавление их в календарь ---
        current_time = 0.0
        for i in range(num_requests):
            # Случайно выбираем клиента для следующей заявки
            client_id = random.randint(1, self.num_clients)
            req = self.client.generate_request(current_time, client_id)
            heapq.heappush(event_calendar, Event(req.generation_time, 'GENERATE', req))
            current_time = req.generation_time # Обновляем для следующей генерации

        # --- Счетчики для статистики ---
        generated_count = 0
        rejected_count = 0
        processed_count = 0
        total_wait_time = 0.0
        total_service_time = 0.0
        # Для статистики по клиентам
        client_stats = {i: {'generated': 0, 'served': 0, 'rejected': 0, 'total_wait_time': 0.0, 'total_service_time': 0.0, 'wait_times': [], 'service_times': []} for i in range(1, self.num_clients + 1)}

        # --- Главный цикл симуляции ---
        while event_calendar:
            current_event = heapq.heappop(event_calendar)
            current_time = current_event.time

            if current_event.type == 'GENERATE':
                generated_count += 1
                req = current_event.data
                client_stats[req.source_id]['generated'] += 1

                # Попытка добавить в буфер (всегда добавляет, может вытеснить)
                # Отказы учитываются внутри add_request при вытеснении
                added, evicted_req = self.buffer.add_request(req)

                # Обновляем статистику для ВЫТЕСНЕННОЙ заявки
                if evicted_req is not None:
                    client_stats[evicted_req.source_id]['rejected'] += 1
                    rejected_count += 1 # Обновляем общий счетчик

                if added:
                    # Проверяем, есть ли свободные специалисты
                    for specialist in self.specialists:
                        if specialist.is_free_at_time(current_time):
                            # Назначаем заявку на специалиста
                            assigned = self.placement_dispatcher.dispatch_to_specialist(req, current_time)
                            if assigned:
                                processed_count += 1
                                client_stats[req.source_id]['served'] += 1
                                wait_time = req.start_service_time - req.generation_time
                                service_time = req.end_service_time - req.start_service_time
                                total_wait_time += wait_time
                                total_service_time += service_time
                                client_stats[req.source_id]['total_wait_time'] += wait_time
                                client_stats[req.source_id]['total_service_time'] += service_time
                                client_stats[req.source_id]['wait_times'].append(wait_time)
                                client_stats[req.source_id]['service_times'].append(service_time)
                                # Добавляем событие освобождения специалиста
                                heapq.heappush(event_calendar, Event(req.end_service_time, 'FREE_CHANNEL', specialist.id))
                            else:
                                # Это маловероятно, но на всякий случай
                                # pass # Не увеличиваем rejected_count здесь
                             break # Назначили, выходим из цикла по специалистам
                # <-- НЕТ else: блока. add_request всегда возвращает True в этой логике.
                # Отказы происходят внутри add_request при вызове remove_last_request.

            elif current_event.type == 'FREE_CHANNEL':
                specialist_id = current_event.data
                specialist = self.specialists[specialist_id - 1] # Индексация с 0
                specialist.free_specialist()

                # Попытка назначить заявку из буфера на освободившийся специалист
                selection_dispatcher = SelectionDispatcher(self.placement_dispatcher, self.buffer)
                assigned, next_req = selection_dispatcher.select_and_dispatch(current_time)
                if next_req:
                    if assigned:
                        processed_count += 1
                        client_stats[next_req.source_id]['served'] += 1
                        wait_time = next_req.start_service_time - next_req.generation_time
                        service_time = next_req.end_service_time - next_req.start_service_time
                        total_wait_time += wait_time
                        total_service_time += service_time
                        client_stats[next_req.source_id]['total_wait_time'] += wait_time
                        client_stats[next_req.source_id]['total_service_time'] += service_time
                        client_stats[next_req.source_id]['wait_times'].append(wait_time)
                        client_stats[next_req.source_id]['service_times'].append(service_time)
                        # Добавляем событие освобождения специалиста
                        heapq.heappush(event_calendar, Event(next_req.end_service_time, 'FREE_CHANNEL', specialist_id))
                    else:
                        # Это маловероятно, но на всякий случай
                        client_stats[next_req.source_id]['rejected'] += 1
                        rejected_count += 1

        # --- Расчет финальной статистики ---
        report = {
            'total_generated': generated_count,
            'total_rejected': rejected_count,
            'total_processed': processed_count,
            'p_current': rejected_count / generated_count if generated_count > 0 else 0.0,
            'client_detailed_stats': client_stats
        }

        if processed_count > 0:
            report['avg_wait_time'] = total_wait_time / processed_count
            report['avg_service_time'] = total_service_time / processed_count
            report['avg_system_time'] = (total_wait_time + total_service_time) / processed_count

            # Расчет дисперсий для общей статистики
            all_wait_times = []
            all_service_times = []
            for stats in client_stats.values():
                all_wait_times.extend(stats['wait_times'])
                all_service_times.extend(stats['service_times'])
            mean_wait = report['avg_wait_time']
            mean_serv = report['avg_service_time']
            disp_wait = sum((t - mean_wait)**2 for t in all_wait_times) / (len(all_wait_times) - 1) if len(all_wait_times) > 1 else 0.0
            disp_serv = sum((t - mean_serv)**2 for t in all_service_times) / (len(all_service_times) - 1) if len(all_service_times) > 1 else 0.0
            report['disp_wait_time'] = disp_wait
            report['disp_service_time'] = disp_serv
        else:
            report['avg_wait_time'] = 0.0
            report['avg_service_time'] = 0.0
            report['avg_system_time'] = 0.0
            report['disp_wait_time'] = 0.0
            report['disp_service_time'] = 0.0

        # Рассчитываем общее время моделирования (время последнего события)
        max_service_end_time = current_time
        for sp in self.specialists:
            if sp.busy_until_time > max_service_end_time:
                max_service_end_time = sp.busy_until_time
        report['total_system_time'] = max_service_end_time

        # Рассчитываем коэффициенты использования специалистов
        report['specialist_loads'] = []
        report['specialist_busy_times'] = []
        for sp in self.specialists:
            report['specialist_busy_times'].append(sp.total_busy_time)
            load = sp.total_busy_time / report['total_system_time'] if report['total_system_time'] > 0 else 0.0
            report['specialist_loads'].append(load)

        return report

    def run(self):
        print(f"Запуск автоматического режима с N={self.N} заявками от {self.num_clients} клиентов...")
        report = self.run_simulation(self.N)
        self.print_final_results(report)

    def print_final_results(self, report):
        print("\n" + "="*50)
        print("РЕЗУЛЬТАТЫ МОДЕЛИРОВАНИЯ (АВТОМАТИЧЕСКИЙ РЕЖИМ)")
        print("="*50)

        # Таблица 1: Характеристики клиентов
        print("\n=== Table 1: Client characteristics ===")
        print("-" * 70)
        print(f"{'№':<5} {'Client':<10} {'Requests':<10} {'p_rej':<8} {'T_stay':<8} {'TBuff':<8} {'Tserv':<8} {'DBuff':<8} {'Dserv':<8}")
        print("-" * 70)
        for client_id, stats in report['client_detailed_stats'].items():
            generated = stats['generated']
            served = stats['served']
            rejected = stats['rejected']
            p_rej = rejected / generated if generated > 0 else 0.0
            avg_wait = stats['total_wait_time'] / served if served > 0 else 0.0
            avg_serv = stats['total_service_time'] / served if served > 0 else 0.0
            avg_stay = avg_wait + avg_serv

            # Расчет дисперсий для конкретного клиента
            wait_times = stats['wait_times']
            service_times = stats['service_times']
            mean_wait_src = avg_wait
            mean_serv_src = avg_serv
            disp_wait_src = sum((t - mean_wait_src)**2 for t in wait_times) / (len(wait_times) - 1) if len(wait_times) > 1 else 0.0
            disp_serv_src = sum((t - mean_serv_src)**2 for t in service_times) / (len(service_times) - 1) if len(service_times) > 1 else 0.0

            print(f"{client_id:<5} {'Client ' + str(client_id):<10} {generated:<10} {p_rej:<8.4f} {avg_stay:<8.4f} {avg_wait:<8.4f} {avg_serv:<8.4f} {disp_wait_src:<8.4f} {disp_serv_src:<8.4f}")

        # Таблица 2: Характеристики специалистов
        print("\n=== Table 2: Statistics by specialists ===")
        print("-" * 40)
        print(f"{'№':<5} {'Specialist':<10} {'Coefficient':<12} {'Working time':<12}")
        print("-" * 40)
        for i, (load, busy_time) in enumerate(zip(report['specialist_loads'], report['specialist_busy_times']), 1):
            specialist_name = f"L{i}"
            print(f"{i:<5} {specialist_name:<10} {load:<12.4f} {busy_time:<12.4f}")

        print(f"\nTotal time occupied: {report['total_system_time']:.2f}")
        print("="*50)


# --- Основной блок ---
if __name__ == "__main__":
    # --- Настройка параметров системы ---
    avg_interval = 0.7# Увеличиваем интенсивность генерации (1/0.6 = 1.67)
    num_clients = 10    # Количество клиентов
    buffer_size = 4    # Размер буфера
    num_specialists = 2 # Количество специалистов
    service_min = 1.0   # Увеличиваем минимальное время обслуживания
    service_max = 1.4   # Увеличиваем максимальное время обслуживания

    # Создание компонентов системы
    client = Client(avg_interval, num_clients)
    buffer = Buffer(buffer_size)
    specialists = [Specialist(i+1) for i in range(num_specialists)]
    placement_dispatcher = PlacementDispatcher(specialists, buffer, service_min, service_max)

    # Создание и запуск симулятора
    simulator = AutoModeSimulator(client, buffer, specialists, placement_dispatcher, fixed_n=5000, num_clients=num_clients)
    simulator.run()