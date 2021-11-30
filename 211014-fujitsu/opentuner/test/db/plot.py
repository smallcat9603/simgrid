import sqlite3
import matplotlib.pyplot as plt
from matplotlib import style
style.use('fivethirtyeight')
import os.path

db = "calc5.db" # assume in the same directory with __file__

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
db_path = os.path.join(BASE_DIR, db)
dbase = sqlite3.connect(db_path)
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