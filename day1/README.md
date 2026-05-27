第一部分：每个命令的核心用法 + 实践任务
1. ls – 列出目录内容
核心功能：查看文件/文件夹信息。

常用选项	含义
-l	长格式（权限、大小、时间）
-a	显示所有文件（包括隐藏文件，以.开头）
-h	人类可读的大小（KB, MB）
-t	按修改时间排序
-r	反向排序
动手任务：

bash
cd ~                     # 回家目录
ls                       # 列出文件名
ls -l                    # 详细信息
ls -a                    # 包含隐藏文件
ls -lh                   # 文件大小显示为K/M/G
ls -ltr                  # 按时间倒序（最新的在最后）
mkdir test_dir           # 创建一个测试目录
touch file1.txt file2.log
ls -l *.txt              # 只显示txt文件
检验：你能看到 test_dir 和两个文件吗？-lh 的大小单位是否变成了K？

2. cd – 切换目录
核心功能：改变当前工作路径。

命令	效果
cd /home	绝对路径切换
cd ..	上一级目录
cd -	上一个工作目录
cd ~ 或 cd	回到用户主目录
动手任务：

bash
cd /tmp                 # 切换到/tmp
pwd                     # 打印当前路径（确认你在/tmp）
cd ~                    # 回到home
cd -                    # 跳回/tmp（注意看终端输出）
cd ..                   # 到根目录/的上一级？不，根目录的上一级还是自己
cd /var/log
cd ~/test_dir           # 进入刚才创建的目录
检验：每次 cd 后用 pwd 确认位置。

3. ps – 查看进程
核心功能：显示当前系统的进程快照。

常用选项	含义
ps aux	显示所有用户的所有进程（BSD风格）
ps -ef	显示所有进程（System V风格）
ps -e --forest	树形显示进程父子关系
动手任务：

bash
ps                       # 只显示当前终端的进程
ps aux | head -5         # 只看前5行，了解各列含义（USER, PID, %CPU等）
ps -ef | grep systemd    # 找到systemd进程的PID
ps -e --forest | less    # 用上下键浏览进程树（按q退出）
检验：你能找到 bash 或 zsh 进程吗？（你当前shell的PID）

4. grep – 文本搜索
核心功能：在文件或输入流中搜索匹配模式的行。

常用选项	含义
-i	忽略大小写
-n	显示行号
-v	反向匹配（显示不包含模式的行）
-r	递归搜索目录
动手任务：

bash
# 准备测试文件
echo -e "apple\nbanana\nApple\norange\nbanana" > fruits.txt

grep banana fruits.txt           # 输出 banana（两次）
grep -i apple fruits.txt         # 输出 apple 和 Apple（忽略大小写）
grep -n orange fruits.txt        # 输出 "4:orange"
grep -v apple fruits.txt         # 输出不包含apple的行（注意大小写）

# 管道用法（最常用）
ps aux | grep ssh                # 找出所有包含ssh的进程
检验：ps aux | grep bash 应该输出你的shell进程。

5. awk – 文本处理（列操作神器）
核心功能：按列处理文本，非常适合提取、计算、格式化。

基础语法：awk '{print $1, $2}' 表示输出每行的第1列和第2列（默认空格/制表符分隔）。

动手任务：

bash
# 准备测试文件
echo -e "name age\nAlice 25\nBob 30\nCharlie 22" > people.txt

# 基本用法
awk '{print $1}' people.txt       # 输出第一列：name, Alice, Bob, Charlie
awk '{print $2}' people.txt       # 输出第二列：age, 25, 30, 22
awk '{print $1, $2}' people.txt   # 输出整行（等效于print $0）

# 条件过滤（第二列大于25的行）
awk '$2 > 25 {print $1}' people.txt   # 输出 Bob

# 与管道结合
ls -l | awk '{print $5, $9}'      # 输出文件大小和文件名（第5列和第9列）
ps aux | awk '{print $1, $11}'    # 输出用户名和命令名（注意$11可能包含参数）
检验：ls -l | awk '{sum+=$5} END {print sum}' 可以计算当前目录所有文件的总大小（字节），试试看。

第二部分：命令组合实战（必做）
真正强大的地方是把这些命令通过管道 | 串联起来。

任务1：找到当前目录下最大的5个文件
bash
ls -lSh | head -6     # -S按大小排序，head取前6行（第一行是total）
任务2：统计当前系统中有多少个不同的bash进程（去重）
bash
ps aux | grep bash | awk '{print $11}' | sort | uniq -c
任务3：查看所有以".log"结尾的进程（比如syslog）
bash
ps -ef | grep -E "\.log$"    # -E允许扩展正则，\.log$匹配以.log结尾
任务4：统计/var/log目录下所有文件中包含"error"的行数（递归）
bash
sudo grep -r "error" /var/log/ | wc -l   # wc -l 统计行数
（注意需要sudo权限）

 第三部分：自测练习题（独立完成）
请打开终端，尝试完成以下任务，并把你的命令和输出结果记录下来（可以在~/cpp_ai_learning/day1/practice.md中记录）。

列出home目录下所有隐藏文件（以点开头），并显示详细信息。

切换到 /usr/bin 目录，查看当前路径，然后返回到上一个目录。

用 ps 命令找出所有 systemd 相关的进程ID（PID）。

从 /etc/passwd 文件中提取所有用户的用户名（第一列）（提示：cat /etc/passwd | awk -F: '{print $1}'）。

用 grep 找出 ~/.zshrc 中所有以 alias 开头的行，并显示行号。

进阶挑战：编写一个命令，统计当前目录下所有文件（不包括目录）的总大小，并以MB为单位显示（提示：ls -l + awk 累加第5列，最后除以1024/1024）。

--------------------------------------------------------------------------------
Git使用总结
1. 安装与配置 Git（如果还没做）
bash
# 1.1 安装 Git
sudo apt update && sudo apt install git -y

# 1.2 设置用户名和邮箱（用于标识提交人）
git config --global user.name "你的GitHub用户名"
git config --global user.email "你的GitHub注册邮箱"

# 1.3 验证配置是否成功
git config --list | grep user
说明：第 1.2 步中，请将 "你的GitHub用户名" 和 "你的GitHub注册邮箱" 换成你自己的真实信息。

2. 生成并配置 SSH 密钥（安全免密登录）
使用 SSH 协议，可以让你推送代码时无需重复输入用户名和密码。

bash
# 2.1 生成 SSH 密钥对（一路按回车即可）
ssh-keygen -t ed25519 -C "你的GitHub注册邮箱"

# 2.2 复制公钥内容（下一步要添加到 GitHub）
cat ~/.ssh/id_ed25519.pub
说明：第 2.1 步中，同样替换邮箱地址。

添加公钥到 GitHub：

登录 GitHub，点击右上角头像 → Settings。

左侧边栏选择 SSH and GPG keys → 点击 New SSH key。

Title 任意填写，Key 粘贴刚刚 cat 命令输出的内容 → 点击 Add SSH key 保存。

验证连接：

bash
ssh -T git@github.com
如果看到类似 Hi xxx! You've successfully authenticated... 的消息，就表示配置成功了。

补充：某些网络环境下（比如公司防火墙限制），默认端口 22 可能不通。如果 ssh -T git@github.com 超时，可以尝试切换到 443 端口，具体方法可以搜索 "GitHub SSH 443 端口配置" 进一步了解。

3. 在 GitHub 上创建你的仓库
登录 GitHub，点击右上角的 + 号 → New repository。

Repository name 输入 cpp-ai-learning。

其余选项保持默认（不要勾选 "Add a README file" 等初始化选项）。

点击 Create repository。

小提示：创建完毕后，你会看到一个 "Quick setup" 页面，暂时不用操作。复制 SSH 地址（格式如 git@github.com:你的用户名/cpp-ai-learning.git），稍后会用到。

4. 初始化本地仓库并关联远程
bash
# 4.1 进入你的学习根目录
cd ~

# 4.2 创建项目文件夹并进入
mkdir -p cpp-ai-learning/day1
cd cpp-ai-learning

# 4.3 初始化 Git 仓库
git init

# 4.4 关联远程仓库（替换成你自己的 SSH 地址）
git remote add origin git@github.com:你的用户名/cpp-ai-learning.git

# 4.5 验证远程关联是否成功
git remote -v
git remote -v 命令应显示 origin 对应的 fetch 和 push 地址。

5. 创建笔记文件（包含今天的命令）
用 nano 编辑器创建并编写 README.md：

bash
# 5.1 进入 day1 目录
cd ~/cpp-ai-learning/day1

# 5.2 创建笔记文件
nano README.md
在编辑器中输入你的学习笔记（下面是一份供参考的模板，建议保留你今天练习时真实的命令和输出）：

markdown
# Day 1 (2026-05-27) 学习笔记

## 今日学习内容
- 命令实战：`ls`、`cd`、`ps`、`grep`、`awk` 基础用法。
- Git 与 GitHub：初始化仓库、SSH 配置、首次推送。
- 环境准备：在 VMware Ubuntu 22.04 中安装并测试了基础工具。

## 练习的命令与输出示例

### 1. `ls` - 列出目录内容
```bash
cd ~
ls -la
输出示例：total 36 drwxr-x--- 5 kris-lee ...

2. cd - 切换目录
bash
cd /tmp
pwd        # 输出 /tmp
cd -       # 返回 home
3. ps + grep - 查找进程
bash
ps aux | grep bash
输出示例：kris-lee 1234 0.0 0.1 12345 6789 pts/0 Ss 10:00 0:00 -bash

4. awk - 文本处理
bash
ls -l | awk '{print $5, $9}'
输出示例：1234 file1.txt 5678 file2.log

今日小结
VMware 网络一度未通，通过检查 NAT 服务并重启解决。成功将第一天的学习笔记推送到 GitHub。

text

保存退出：按 `Ctrl+O`，回车确认，再按 `Ctrl+X`。

---

### 6. 将笔记提交并推送到 GitHub

```bash
# 6.1 回到仓库根目录
cd ~/cpp-ai-learning

# 6.2 添加所有文件到暂存区
git add .

# 6.3 提交并附上说明（-m 后的内容是提交信息，可自定义）
git commit -m "Add day1 learning notes: linux commands"

# 6.4 首次推送（建立本地 main 分支与远程仓库 main 分支的关联）
git push -u origin main
推送成功后，刷新你的 GitHub 仓库页面，day1/README.md 应该就已经显示出来了。

7. 验证上传结果
在终端中执行以下命令确认状态：

bash
git status
如果显示 nothing to commit, working tree clean，说明所有更改都已提交。

前往你的 GitHub 仓库页面（https://github.com/你的用户名/cpp-ai-learning），查看 day1/README.md 文件是否存在，内容是否符合预期。

常见问题与解决方案
git push 时报 Updates were rejected because the remote contains work that you do not have locally

原因：远程仓库有本地没有的提交（通常是因为 GitHub 上创建仓库时勾选了初始化选项，如 README、.gitignore）。

解决方案：先拉取远程内容并合并，再推送。

bash
git pull origin main --allow-unrelated-histories
git push origin main
ssh: connect to host github.com port 22: Connection timed out

原因：网络防火墙限制，导致默认 22 端口不通。

解决方案：可以尝试切换到 443 端口。方法是编辑 ~/.ssh/config 文件（如果不存在则新建），添加以下内容：

text
Host github.com
  Hostname ssh.github.com
  Port 443
  User git
然后再次执行 ssh -T git@github.com 验证。

如何删除错误的提交重新推送

如果提交错了想撤销最近一次 commit：

bash
git reset --soft HEAD~1   # 撤销 commit，但保留修改在暂存区
git commit -m "新的提交信息"
git push origin main --force
注意：--force 会覆盖远程历史，对于个人学习仓库可以使用，但在多人协作的项目中务必谨慎。


