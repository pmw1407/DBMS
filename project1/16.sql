SELECT SUM(sumOfTypes)
FROM (SELECT type, COUNT(*) as sumOfTypes
      FROM Pokemon
      WHERE type = 'Water' OR type = 'Electric' OR type = 'Psychic'
      GROUP BY type) AS result;