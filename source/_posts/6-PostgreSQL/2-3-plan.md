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
## 1.1 生成计划的目的
上文提到，在语法分析、语义分析之后，PostgreSQL已获取执行所需的详细信息。计划生成主要针对一些复杂的SQL语句，主要是SQL语句，计划器主要告诉PostgreSQL，如何更高效地执行复杂的SQL语句：
- 示例1
假设，有1个表，表上有1个索引，表中有1000万条数据
```sql
CREATE TABLE t1 (c1 INT, c2 TEXT);
CREATE INDEX i1 ON t1(c1);
INSERT INTO t1 .. -- 假设1000万条数据
```
当用户执行以下SQL语句时，数据库需要从t1表中，依次扫描所有的数据，此时，索引并没有什么作用：
```sql
SELECT * FROM t1;
```
当用户执行以下数据时，数据有2种执行方案：
- 语句
    ```sql
    SELECT * FROM t1 WHERE c1 = 10000;
    ```
- 方案1：依次扫描t1表所有数据，然后找到c1=10000的几行
- 方案2：先扫描索引i1，找到c1=10000的几行数据所在的位置，再去t1表中指定位置读取数据
显然，上述示例中，方案2的执行速度更快，数据库需要根据一定的策略，选择哪种执行方案。这便是计划器的职责。

## 1.2 生成计划的场景

## 1.3 计划的简单表示

## 1.4 算子的分类

# 2 算子
## 2.1 扫描算子
### 问题场景
### 一、SeqScan
```bash
| PageIdx | Tuple Idx | c1 |   c2  |
+---------+-----------+----+-------+
|         |     1     |  6 | data6 |
|   1     |     2     |  8 | data8 |
|         |     3     |  4 | data4 |
+---------+-----------+----+-------+
|         |     1     |  9 | data9 |
|   2     |     2     |  1 | data1 |
|         |     3     |  8 | data8 |
+---------+-----------+----+-------+
```

### 二、IndexScan
1. 前置条件：存储数据
```bash
| PageIdx | Tuple Idx | c1 |   c2  |
+---------+-----------+----+-------+
|         | 1 [delete]|  6 | data6 |
|   1     |     2     |  8 | data8 |  存在被删除的数据
| [delete]|     3     |  4 | data4 |
+---------+-----------+----+-------+----
|         |     1     |  9 | data9 |
|   2     |     2     |  1 | data1 |  不存在被删除的数据
|         |     3     |  8 | data8 |
+---------+-----------+----+-------+

        | PageIdx | Tuple Idx | c1 |  tid  |
        +---------+-----------+----+-------+
        |         |     1     |  1 | [2,2] |
        |   1     |     2     |  4 | [1,3] |
        |         |     3     |  6 | [1,1] |
        +---------+-----------+----+-------+
                                            \
                                             \
                                | PageIdx | Tuple Idx | c1 |  tid  |
                                +---------+-----------+----+-------+
                                |         |     1     |  8 | [1,2] |
                                |   2     |     2     |  8 | [2,3] |
                                |         |     3     |  9 | [2,1] |
                                +---------+-----------+----+-------+
```
2. 查询数据
```sql
SELECT * FROM t1 WHERE c1 > 6 AND c1 < 9;
```
3. 查找数据
    1. 在Index中，找到c1=8的数据对应的tid=[1,2]
    2. 在Heap中，读取1个Page，获取第2个Tuple
    3. 在Index中，找到c1=8的数据对应的tid=[2,3]
    4. 在Heap中，读取2个Page，获取第3个Tuple
4. 返回数据
```bash
 c1 | c2 
----+--------
 8  | data8
 8  | data8
```

### 三、IndexOnlyScan
1. 前置条件：存储数据
```bash
| PageIdx | Tuple Idx | c1 |   c2  |
+---------+-----------+----+-------+
|         | 1 [delete]|  6 | data6 |
|   1     |     2     |  8 | data8 |  存在被删除的数据
| [delete]|     3     |  4 | data4 |
+---------+-----------+----+-------+----
|         |     1     |  9 | data9 |
|   2     |     2     |  1 | data1 |  不存在被删除的数据
|         |     3     |  8 | data8 |
+---------+-----------+----+-------+

        | PageIdx | Tuple Idx | c1 |  tid  |
        +---------+-----------+----+-------+
        |         |     1     |  1 | [2,2] |
        |   1     |     2     |  4 | [1,3] |
        |         |     3     |  6 | [1,1] |
        +---------+-----------+----+-------+
                                            \
                                             \
                                | PageIdx | Tuple Idx | c1 |  tid  |
                                +---------+-----------+----+-------+
                                |         |     1     |  8 | [1,2] |
                                |   2     |     2     |  8 | [2,3] |
                                |         |     3     |  9 | [2,1] |
                                +---------+-----------+----+-------+
```
2. 查询数据
必要条件：要查找的列，全在索引文件中
```sql
SELECT c1 FROM t1 WHERE c1 > 6 AND c1 < 9;
```
3. 查找数据
    1. 从索引中，基于B+树的原理，快速找到c1=6的Tid为[1,1]
    2. 从Vm文件中，判断第1个表Page是否包含被标记删除的Tuple，发现1第个Page，存在被删除的Tuple
    2. 读取表文件的第1个Page，然后找到第1个Tuple
    3. 

### 四、BitmapScan
3. 查找数据
    1. 找到需要读取的Tuple [1,1] [1,2] [2,3]
    2. 读取第1个Page，获取第1和第2个Tuple
    2. 读取第2个Page，获取第3个Tuple

### 五、SubquryScan
### 六、

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