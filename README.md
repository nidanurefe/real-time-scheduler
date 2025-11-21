# Real-Time Scheduling Simulator (C++)

### Supported Algorithms:
- **EDF** – Earliest Deadline First  
- **RMS** – Rate Monotonic Scheduling  
- **DMS** – Deadline Monotonic Scheduling  
- **LLF** – Least Laxity First  
- **Background** – Background server  
- **Polling Server**  
- **Deferrable Server**  
- **Sporadic Server**

  
It parses a task input file, runs the selected scheduling algorithm, and prints a complete scheduling timeline, Gantt-like visualization, finished jobs, and deadline misses.

## Project Structure: 
```text
scheduler/
│
├── CMakeLists.txt          # CMake yapı betiği
├── src/
│   ├── main.cpp            # CLI 
│   ├── models.hpp          # Task / Job / ServerCfg data models, hyperperiod
│   ├── parser.hpp          # parser
│   ├── policies.hpp        # EDF, RMS, DMS, LLF priority politics
│   ├── sched_base.hpp      # Periodic scheduler + Background scheduler
│   ├── sched_servers.hpp   # Polling, Deferrable, Sporadic server schedulers
│   └── factory.hpp         # Creates proper scheduler 
│
└── examples/
    └── example.in
```

## Building the Project:

1 - Create build directory
```
mkdir build && cd build
```


2 - Run CMake
```
cmake ..
```

3 - Compile
```
make
```

4 - Output executable: 
```
./rt_scheduler
```
