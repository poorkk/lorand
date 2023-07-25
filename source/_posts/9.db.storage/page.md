---
title: 存储引擎 page
date: 2022-09-25 16:03:37
categories:
    - 存储引擎
tags:
    - 存储引擎
---

PageInit()
```c
/* wu  */

```
XLOG_FPI_FOR_HINT
XLOG_FPI

BKPBLOCK_HAS_IMAGE
    REGBUF_FORCE_IMAGE
        log_newpage()
        XLogSaveBufferForHint()
            MarkBufferDirtyHint()

needs_backup = (page_lsn <= RedoRecPtr);