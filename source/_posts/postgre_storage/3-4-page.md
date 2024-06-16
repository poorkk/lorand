---
title: PostgreSQL 3-4 Page
date: 2022-09-25 16:34:37
categories:
    - PostgreSQL
tags:
    - PostgreSQL
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