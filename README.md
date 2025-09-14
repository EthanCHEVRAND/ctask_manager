# ctask_manager
A minimal task manager for everyday life in cli (in C)

## Features

- Add a task
- List tasks (pending / done)
- Mark tasks done
- Remove a task

## Data storing
Tasks are stored in 
'/home/USER/.local/share/ctask_manager/tasks.json'.

```json
[{
    "id": 1, 
    "done": false, 
    "created_at": "13/09/2025 - 07:12", 
    "desc": "Buy milk", 
    "priority": 2, 
    "tags": [ "groceries", "irl" ] 
}]
```

## Build
```bash
make
```
This creates 'bin/task'.

## Installation
```bash
git clone https://github.com/EthanCHEVRAND/ctask_manager.git
```

```bash
./install.sh
```
This creates everything needed for the tool to work, so no need to run make alongside it.

## Usage
```bash
ctask_man usage
```

```bash
ctask_man add "Task" [-p <1-3>] [-t <tags>]
ctask_man list [-a --all]
ctask_man done <task_id>
ctask_man rm <task_id>
```
