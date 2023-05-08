
初始化Buffer池
```c
InitBufferPool()
    BufferDescriptors = ShmemInitStruct()
    BufferBlocks = ShmemInitStruct()
    StrategyInitialize()
```

```c
ServerLoop()
    StartBackgroundWriter()
        StartChildProcess()
            AuxiliaryProcessMain()
                BackgroundWriterMain()         
```

```c
BackgroundWriterMain()
    // 1 内存上下文你
    // 2 注册错误长跳转
    for (;;) {
        ResetLatch()
       
        BgBufferSync()
            StrategySyncStart()
            while (num_to_scan > 0 && reusable_buffers < upcoming_alloc_est)
                SyncOneBuffer()
                    FlushBuffer(bufHdr, NULL);
        pgstat_send_bgwriter()

        WaitLatch()
            WaitLatchOrSocket()
               select() 
   }
```