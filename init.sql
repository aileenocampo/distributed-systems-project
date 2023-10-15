CREATE TABLE IF NOT EXISTS mp3Files (
    id INT AUTO_INCREMENT PRIMARY KEY,
    songName VARCHAR(255) NOT NULL,
    link VARCHAR(1024) NOT NULL
);

INSERT INTO mp3Files (songName, link) VALUES 
('Mine', 'https://www.dropbox.com/scl/fi/kktn6lrpce2itc646tpxd/01-Mine-Taylor-s-Version.mp3?rlkey=vez8zo2ydlxo6mw4dn8fzprpx&dl=1'),
('Sparks Fly', 'https://www.dropbox.com/scl/fi/ib0myem7ndap84vyw9aid/02-Sparks-Fly-Taylor-s-Version.mp3?rlkey=3q7l5mq4chifvm4gsoy8ggcsc&dl=1'),
('Back to December', 'https://www.dropbox.com/scl/fi/7mmk70n2tzn6ons27ouj7/03-Back-to-December-Taylor-s-Version.mp3?rlkey=fdgqnk3lx8lxygohfe5jwxiq0&dl=1'),