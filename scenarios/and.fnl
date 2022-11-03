(local labels [])
(table.insert labels "A")
(print (length labels))
(table.insert labels "B")
(table.insert labels "C")
(print (length labels))

(let [labels []]
    (table.insert labels "A")
    (print (length labels))
    (table.insert labels "B")
    (print (length labels))
)