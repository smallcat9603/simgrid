import sqlite3
import matplotlib.pyplot as plt
from matplotlib import style
style.use('fivethirtyeight')

dbase = sqlite3.connect("test.db")
cursor = dbase.cursor()
print("database opened")
print("cursor created")

dbase.execute(''' CREATE TABLE IF NOT EXISTS employee_records(
    ID INT PRIMARY KEY NOT NULL,
    NAME TEXT NOT NULL,
    DIVISION TEXT NOT NULL,
    STARS INT NOT NULL
)
''')
print("table created")

def insert_record(ID, NAME, DIVISION, STARS):
    dbase.execute(''' INSERT INTO employee_records(ID, NAME, DIVISION, STARS)
        VALUES(?, ?, ?, ?)
    ''',(ID, NAME, DIVISION, STARS))
    dbase.commit()
    print('record inserted')

# insert_record(6, 'Bob', 'Hardware', 4)

def read_data():
    data = dbase.execute(''' SELECT * FROM employee_records ORDER BY NAME
    ''')
    for record in data:
        print("ID: " + str(record[0]))
        print("NAME: " + str(record[1]))
        print("DIVISION: " + str(record[2]))
        print("STARS: " + str(record[3]))
        print()

read_data()

def update_record():
    dbase.execute(''' UPDATE employee_records set STARS=3 WHERE ID=6
    ''')
    dbase.commit()
    print("updated")

# update_record()
# print("-----")
# read_data()

def delete_record():
    dbase.execute(''' DELETE from employee_records WHERE ID = 6 
    ''')
    dbase.commit()
    print("deleted")

# delete_record()
# print("-----")
# read_data()

def check_data():
    data = cursor.execute(''' SELECT NAME FROM employee_records WHERE ID = 50 
    ''')
    x = data.fetchall()
    if x == []:
        print("not exist")
    else:
        print(x)

check_data()

def graph_data():
    cursor.execute('SELECT NAME, STARS FROM employee_records')
    data = cursor.fetchall()

    names = []
    stars = []
    
    for row in data:
        names.append(row[0])
        stars.append(row[1])

    plt.plot(names,stars,'-')
    plt.show()

graph_data()

dbase.close()
print("database closed")