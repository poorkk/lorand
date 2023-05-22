---
title: SQL引擎 优化器
date: 2023-01-01 16:03:37
categories:
    - SQL引擎
tags:
    - SQL引擎
---

# 1 概述
## 1.1 整体流程
```c
exec_simple_query()
    pg_parse_query()
    for (;;):
        pg_analyze_and_rewrite()
        pg_plan_queries()
        PortalRun()
```

## 1.2 计划器
```c
pg_plan_queries()
    pg_plan_query(Query)
        PlannedStmt *plan  = planner(Query)
            standard_planner(Query)
                PlannerGlobal *glob = makeNode(PlannerGlobal)
                subquery_planner(PlannerGlobal *glob, Query *parse, PlannerInfo **subroot)
                    PlannerInfo *root = makeNode(PlannerInfo)
                    root->parse = parse
                    if parse->cteList:
                        SS_process_ctes(root)
                    if parse->hasSubLinks:
                        pull_up_sublinks(root)
                    inline_set_returning_functions(root)
                    pull_up_subqueries(root)
                    if parse->setOperations:
                        flatten_simple_union_all(root)
                    preprocess_rowmarks(root)
                    xpand_inherited_tables(root)
                    parse->targetList = preprocess_expression(root, parse->targetList)
                    parse->returningList = preprocess_expression(root, parse->returningList)
                    preprocess_qual_conditions(root, parse->jointree)
                    parse->havingQual = preprocess_expression(root, parse->havingQual)
                    parse->limitOffset = preprocess_expression(root, parse->limitOffset)
                    parse->limitCount = preprocess_expression(root, parse->limitCount)
                    root->append_rel_list  = preprocess_expression(root, root->append_rel_list)
                    ...
                    if hasOuterJoins:
                        reduce_outer_joins(root)
                    
                    /* do the main plainning */
                    if parse->resultRelation:
                        plan = inheritance_planner(root)
                    else:
                        plan = grouping_planner(root)
                        if parse->commandType != CMD_SELECT:
                            plan = make_modifytable(root, plan, SS_assign_special_param(root))
                    return plan
```

## 1.3 计划器执行流程
```c
grouping_planner(PlannerInfo *root)
    Query *parse = root->parse
    if parse->limitCount || parse->limitOffset:
        preprocess_limit(root)
    if parse->setOperations:
        List *set_sortclauses
        Plan *result_plan = plan_set_operations(root, &set_sortclauses)
        List *current_pathkeys = make_pathkeys_for_sortclauses(root, set_sortclauses, result_plan->targetlist)
        parse->targetList = postprocess_setop_tlist(result_plan->targetlist)
        root->sort_pathkeys = make_pathkeys_for_sortclauses(root, parse->sortClause)
    else:
        ...
        parse->targetList = preprocess_targetlist(root, parse->targetList)
        expand_security_quals(root, parse->targetList)
        ...
        RelOptInfo *final_rel = query_planner(root, standard_qp_callback)
        path_rows = clamp_row_est(final_rel->rows)
        ...
        sorted_path = get_cheapest_fractional_path_for_pathkeys()
        
```