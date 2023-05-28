import time
import requests
from concurrent.futures import ThreadPoolExecutor

def make_request(url):
    response = requests.get(url)
    return response.elapsed.total_seconds()

# 测试的URL
url = "http://localhost:8080"

# 并发请求数量
num_requests = 100000

# 创建线程池
with ThreadPoolExecutor(max_workers=num_requests) as executor:
    # 提交任务到线程池
    futures = [executor.submit(make_request, url) for _ in range(num_requests)]

    # 等待所有任务完成
    total_time = 0.0
    num_completed_requests = 0

    start_time = time.time()

    for future in futures:
        total_time += future.result()
        num_completed_requests += 1

    end_time = time.time()

    # 输出测试结果
    avg_response_time = total_time / num_completed_requests
    total_time_elapsed = end_time - start_time
    qps = num_completed_requests / total_time_elapsed

    print(f"Avg. Response Time: {avg_response_time:.4f} seconds")
    print(f"Total Requests: {num_completed_requests}")
    print(f"Total Time Elapsed: {total_time_elapsed:.2f} seconds")
    print(f"QPS: {qps:.2f}")
