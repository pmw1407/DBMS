SELECT result.id, result.name, two.name, three.name 
FROM (SELECT name, id, E.after_id AS second, V.after_id AS third
  FROM Evolution AS E, Pokemon AS P, Evolution AS V
  WHERE P.id = E.before_id AND E.after_id = V.before_id) AS result, Pokemon AS two, Pokemon AS three
WHERE two.id = result.second AND three.id = result.third
ORDER BY result.id;