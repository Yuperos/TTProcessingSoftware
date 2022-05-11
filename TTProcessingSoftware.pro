TEMPLATE = subdirs

SUBDIRS += \
    Libs \
    User \
    Server

Server.depends = Libs
User.depends = Libs


