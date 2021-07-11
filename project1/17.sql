SELECT COUNT(*)
FROM (SELECT DISTINCT pid 
      FROM Trainer AS T, CatchedPokemon as C
      WHERE T.id = C.owner_id AND T.hometown = 'Sangnok City'
      GROUP BY pid) AS result;
