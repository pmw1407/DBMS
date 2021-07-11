SELECT name
FROM Pokemon as P, Evolution as E
WHERE P.id = E.before_id AND P.type = 'Grass'