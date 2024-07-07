---
title: PostgreSQL 2-2 Analyze
date: 2022-09-25 16:22:37
categories:
    - PostgreSQL
tags:
    - PostgreSQL
---

# 1 概述
## 1.1 整体流程
```c
// 语义分析
exec_simple_query
    pg_parse_query
    pg_analyze_and_rewrite
        parse_analyze
            transformTopLevelStmt
                transformOptionalSelectInto
                    transformStmt   
                        transformInsertStmt

exec_simple_query()
    pg_parse_query()
    for (;;):
        pg_analyze_and_rewrite()
                Query *query = parse_analyze(Node *parsetree)
                List *querytree_list = pg_rewrite_query(query)
        pg_plan_queries()
        PortalRun()
```

## 1.2 分析器
```c
Query *parse_analyze(Node *parseTree)
    ParseState *pstate = make_parsestate()
    Query *query = query = transformTopLevelStmt(pstate, parseTree)
        query = transformStmt(pstate, parseTree) /* 对各种类型语法进行语义分析 */
            Query *result
            switch nodeTag(parseTree) {
                case T_InsertStmt:
                    result = transformInsertStmt(pstate, (InsertStmt *) parseTree)
                case T_DeleteStmt:
                    result = transformDeleteStmt(pstate, (DeleteStmt *) parseTree)
                case T_UpdateStmt:
                    result = transformUpdateStmt(pstate, (UpdateStmt *) parseTree)
                case T_SelectStmt:
                    SelectStmt *n = (SelectStmt *) parseTree;
                        if n->valuesLists:
                            transformValuesClause(pstate, n)
                        else if n->op == SETOP_NONE:
                            result = transformSelectStmt(pstate, n)
                case T_DeclareCursorStmt:
                    ...
                case T_ExplainStmt:
                    ...
                case T_CreateTableAsStmt:
                    ...
                default:
                    result = makeNode(Query)
                    result->commandType = CMD_UTILITY
                    result->utilityStmt = (Node *) parseTree
            }
            return result;
```

## 1.3 SELECT 分析
```c
Query *transformSelectStmt(ParseState *pstate, SelectStmt *stmt)
    Query *qry = makeNode(Query)
    if stmt->withClause:
        qry->cteList = transformWithClause(pstate, stmt->withClause)
    transformFromClause(pstate, stmt->fromClause)
    qry->targetList = transformTargetList(pstate, stmt->targetList)
    markTargetListOrigins(pstate, qry->targetList)
    Node *qual = transformWhereClause(pstate, stmt->whereClause)
    qry->havingQual = transformWhereClause(pstate, stmt->havingClause)
    qry->sortClause = transformSortClause(pstate, stmt->sortClause, &qry->targetList)
    qry->groupClause = transformGroupClause(pstate, stmt->groupClause, &qry->groupingSets, &qry->targetList, qry->sortClause)
    qry->distinctClause = transformDistinctOnClause(pstate, stmt->distinctClause, &qry->targetList, qry->sortClause)
    qry->limitOffset = transformLimitClause(pstate, stmt->limitOffset)
    qry->limitCount = transformLimitClause(pstate, stmt->limitCount)
    qry->windowClause = transformWindowDefinitions(pstate, pstate->p_windowdefs, &qry->targetList)
    qry->jointree = makeFromExpr(pstate->p_joinlist, qual)
    if pstate->p_hasAggs || qry->groupClause || qry->groupingSets || qry->havingQual:
        parseCheckAggregates(pstate, qry)
    assign_query_collations(pstate, qry)
    return qry
```