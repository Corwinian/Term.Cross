#!/bin/bash

# Использование svn для модификации программы TERM.CROSS
#    (частично взято из http://wiki.linuxformat.ru/index.php/LXF70:Subversion2)


#Получение вашей собственной копии исходных текстов TERM.CROSS для пользователя SD
#    trunk - рабочая ветка (eng)
USER=SD
svn checkout svn+ssh://${USER}@toolbox.satellite.dvo.ru/var/svn/lab34/TERM.CROSS/trunk --username ${USER}
# После выполнения команды появляется в рабочей директории появляется директория trunk

# После внесения изменений выполняем (в директории trunk)
svn commit

# для того, чтобы добавить в дерево файл или директорию делаем  (в директории trunk)
svn add ./file_name

# для того, чтобы убрать из дерева файл или директорию делаем  (в директории trunk)
svn remove ./file_name
