import sqlite3
import matplotlib.pyplot as plt
from matplotlib import style
style.use('fivethirtyeight')

dbase = sqlite3.connect("calc5.db")
cursor = dbase.cursor()

def graph_data():
    cursor.execute(''' 
    SELECT id, time FROM result 
    ''')
    data = cursor.fetchall()
    ids = []
    times = []
    for row in data:
        ids.append(row[0])
        times.append(row[1])
    plt.plot(ids,times,'-')
    plt.show()

graph_data()
dbase.close()