# 中山大学2021学年春季操作系统课程
<center>
    任课老师：陈鹏飞
<br>    
    助教：陈泓仰、谢文欣、傅禹泽、张钧宇
</center>

# 目录说明

+ `labX`是第`X`次实验的指导材料、代码和PPT的文件夹。
+ `labX/README.md`是第`X`次实验的指导、要求和提交时间的文档。

# git的使用

## 方法一

每次点击`下载ZIP`将整个项目下载下来。

![git使用方法1](images/git使用方法1.PNG)

## 方法二

> 在此之前，同学们需要安装git，详情参考廖雪峰的git教程[https://www.liaoxuefeng.com/wiki/896043488029600]

由于方法一每次都会重复下载历史的文件，非常麻烦，方法二则提供了git的使用方法。如果还未在本地建立git连接，则需要新建一个文件夹，例如`oslab`。然后在`oslab`下初始化git

```shell
git init
```

创建了git仓库后，我们建立远程仓库连接。

```shell
git remote add gitee git@gitee.com:nelsoncheung/sysu-2021-spring-operating-system.git
```

此后，当远程仓库更新时，我们就从远程仓库中拉取最新的内容。

```shell
git pull gitee main
```

**注意，我们的项目默认发布在`main`分支上，不是`master`分支。**

