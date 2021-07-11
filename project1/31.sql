SELECT type
FROM(SELECT type, COUNT(type) AS num
  FROM Evolution AS E, Pokemon AS P
  WHERE E.before_id = P.id
  GROUP BY type) AS result
WHERE num >= 3
ORDER BY type DESC;