工具类：Utils,GitliteException
核心模块：Object,Branch,Blob,Commit,Repository,SomeObj,StagingArea,Tree 涉及比较通用的部分
命令类：所有command后缀 具体对命令的实现，里面的函数是相对特化的

1. Object 基类

职责：所有Git对象的抽象基类

核心设计：

提供统一的serialize()、getOid()、save()接口

静态方法load()根据OID动态加载不同类型的对象

使用模板特化实现Blob和Commit的反序列化

持久化路径：.gitlite/objects/{前2位OID}/{后38位OID}

2. Blob类

职责：存储文件内容

实例变量：

content: vector<uint8_t> - 文件的原始字节数据

序列化格式：blob {内容长度}\0{文件内容}

工作原理：

直接存储文件原始内容，不压缩

SHA-1哈希基于"blob {size}\0{content}"计算

相同内容生成相同OID，实现内容寻址

3. Commit类

职责：表示一次代码提交

实例变量：

parents_: vector<string> - 父提交OID（支持合并提交）

message_: string - 提交信息

file_blobs_: map<string,string> - 文件名到Blob OID的映射

timestamp_: string - UTC时间戳

序列化格式：

commit {大小}\0
tree {tree_oid}
parent {parent_oid}
author Gitlite <gitlite@example.com> {时间戳}
committer Gitlite <gitlite@example.com> {时间戳}

{提交信息}

复杂任务处理：

获取当前提交：解析HEAD文件→分支引用→提交OID

合并冲突检测：对比两个父提交的文件差异，检测同一文件的不同修改

Tree创建：基于父提交tree+暂存区修改+删除文件生成新tree

4. Tree类

职责：表示提交时的目录结构

实例变量：

entries_: map<string,string> - 文件名:Blob OID映射

sha1_: string - 缓存的SHA-1值

工作原理：

通过createTreeFromParentAndStaging()整合暂存区变更

序列化格式：每行文件名:blob_oid

修改条目后自动重新计算SHA-1

5. Branch类

职责：管理分支引用

静态方法：

create()：基于当前提交创建新分支引用

checkout()：更新HEAD指向，加载对应提交

getCurrentBranch()：解析HEAD中的ref: refs/heads/xxx

边界情况：

不能删除当前分支

检查分支是否存在再操作

重命名分支时更新HEAD（如果是当前分支）

6. StagingArea类

职责：管理暂存区（索引）

目录结构：

.gitlite/staging/ - 暂存文件副本

.gitlite/staging/delete/ - 待删除标记文件

工作原理：

stageFile()：复制文件到staging目录

markFileForDeletion()：在delete目录创建空标记文件

clear()：提交后清空暂存区

状态查询：检查文件在staging或delete目录的存在性

7. Repository类

职责：仓库路径管理

核心功能：

提供.gitlite目录的绝对路径

管理objects、refs等子目录路径

确保目录结构完整性

8. SomeObj类

职责：命令行接口封装

设计：将各模块功能封装为简单的命令方法，如init(), add(), commit()等

