# distributed-systems-project
Docker Initialization: 

docker build -t my-mysql .

docker run --name my-mysql-container -d -p 3306:3306 my-mysql

docker exec -it my-mysql-container mysql -u root -p

password: password
