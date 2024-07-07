---
title: PostgreSQL 2-3 Plan
date: 2022-09-25 16:23:37
categories:
    - PostgreSQL
tags:
    - PostgreSQL
---

# SQL 引擎

# 1 概述
## 1.1 概述
**优化方法**
- 逻辑优化：RBO：基于规则：关系代数，等价变化
- 物理优化：CBO：查询优化：物理执行路径

## 1.2 逻辑优化
**关系代数**
- 选择
- 投影
- 笛卡尔积
- 并集
- 差集

**非关系代数**
- 聚集
- 分组

**常用方法**
- 选择操作下推
- 叶子节点投影
  
## 1.2 物理优化
**四个法宝**
- b+树
- hash表
- 排序
- 物化

**物理路径搜索**
- 自底向上：动态规划
- 自顶向下：枚举
- 随机搜索

**postgresql**
- 表少：动态规划
- 表多：遗传算法

# 2 查询树
## 结构体
- Var
- RangeTblEntry / RangleTblRef
- JoinExpr
- FromExpr
- Query
  
## 函数
- query_tree_mutator / query_tree_walker 
- walker：遍历查询树：修改节点值，但不会增加或删除节点
- mutator：增加或删除节点
  
# 3 逻辑重写优化
## 3.2 提升子查询
**查询**
- 子连接 sublink：以表达式形式存在
    - WHERE / ON 子句 （伴随 ANY/ALL/IN/EXISTS/SOME 等谓语）
    - 投影 子句
- 子查询 subquery：范围表形式
    - FROM 子句

**子查询**
- 相关子查询：子查询引用外层表列属性：外层表执行一次，子查询执行一次 （可提升）
- 非相关子查询：子查询语句独立，外表重复利用子查询结果

## 3.2.1 提升子链接
子链接类型
- EXISTS_SUBLINK （提升）
- ALL_SUBLINK
- ANY_SUBLINK (提升)





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