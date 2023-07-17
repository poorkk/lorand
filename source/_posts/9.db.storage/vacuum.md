---
title: 存储引擎 vacuum
date: 2022-09-25 16:03:37
categories:
    - 存储引擎
tags:
    - 存储引擎
---

1. 启动autovacuum进程
```c
standard_ProcessUtility(Node *parsetree)
    case T_VacuumStmt:
        ExecVacuum(VacuumStmt *vacstmt)
            vacuum(RangeVar *relation, Oid relid, VacuumParams *params)

ServerLoop(void)
    StartAutoVacLauncher(void)
        AutoVacLauncherMain()
            /* if fail: 发送信号重启aotuvacuum进程 */
            SendPostmasterSignal(PMSIGNAL_START_AUTOVAC_WORKER)

sigusr1_handler(SIGNAL_ARGS)
    if CheckPostmasterSignal(PMSIGNAL_START_AUTOVAC_WORKER):
        StartAutovacuumWorker(void)
            Backend *bn = malloc();
            bn->pid = StartAutoVacWorker(void)
                AutoVacWorkerMain(int argc, char *argv[])()
                    BaseInit();
                    InitPostgres(NULL, dbid, NULL, InvalidOid, dbname)
                    do_autovacuum()
                        autovacuum_do_vac_analyze(autovac_table *tab)
                            vacuum(RangeVar *relation, Oid relid, VacuumParams *params)
                    proc_exit(0)
            dlist_push_head(&BackendList, &bn->elem)
````

2. 
```c
do_autovacuum(void) /* table-by-table */
    
```