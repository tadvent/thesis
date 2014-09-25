#-------------------------------------------------------------------------------
# Name:        DrawSchedule
# Purpose:
#
# Author:      tadvent
#
# Created:     15/12/2012
# Copyright:   (c) tadvent 2012
# Licence:     <your licence>
#-------------------------------------------------------------------------------
import os
from matplotlib.pyplot import *
import matplotlib.patches as patches

ax = figure().add_subplot(111)
n = 0

def add_rect(left, right, bottom, height, text, color):
    ax.add_patch(patches.Rectangle((left, bottom),right-left,height,
##                 edgecolor='none',
                 facecolor=color))
    ax.text((left+right)/2, bottom+height/2, text,
            horizontalalignment='center',
            verticalalignment='center')

def draw_proc_sched(f, y, proc_name):
    global n
    s = f.readline()
    while(s != '' and s.strip() == ''):
        s = f.readline()
    N = int(s)
    for i in range(N):
        ll = f.readline().split()
        start = float(ll[0])
        end = float(ll[1])
        text = ll[2]
        iTask = float(text[1:-1].split(',')[0])
        add_rect(start, end, y, 0.8, text, (1.0-iTask/(2*n), 0.5+iTask/(2*n), 0.8, 1))
    ax.text(0, y+0.4, proc_name,
            horizontalalignment='right',
            verticalalignment='center')

def draw_link_sched(f, y):
    s = f.readline()
    while(s != '' and s.strip() == ''):
        s = f.readline()
    p1,p2,n = [i for i in map(int, s.strip().split())]
    if(p1 == 0):
        return False
    for i in range(n):
        ll = f.readline().split()
        start = float(ll[0])
        end   = float(ll[1])
        fromn = ll[2]
        ton   = ll[3]
        fromp = int(ll[4])
        top   = int(ll[5])
        data  = int(ll[6])
        text = "%s\n--->\n%s" % (fromn, ton)
        add_rect(start, end, y, 0.8, text, 'white')
    ax.text(0, y+0.4, 'Link %d -- %d' % (p1, p2),
            horizontalalignment='right',
            verticalalignment='center')
    return True

def draw_from_file(filename):
    global n
    with open(filename, 'r') as f:
        n = int(f.readline())   # number of tasks
        m = int(f.readline())   # number of processors
        for iProc in range(1, m+1):
            draw_proc_sched(f, float(iProc), "Proc %d" % iProc)

        y = m + 1
        while(draw_link_sched(f, y)):
            y = y + 1

        s = f.readline().split()
        ax.set_xlim(0, float(s[0]))
        ax.set_ylim(0, float(s[1]))

def main():
    retcode = os.system('isdf.exe')
    print('return code: ', retcode)
    if(retcode != 0):
        return 1
    draw_from_file('data.txt')
    ax.axes.get_yaxis().set_visible(False)
##    ax.set_frame_on(False)
    show()

if __name__ == '__main__':
    main()
