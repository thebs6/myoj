CREATE TABLE `User` (
  `_id` BIGINT PRIMARY KEY,
  `Avatar` VARCHAR(255),
  `NickName` VARCHAR(255),
  `Account` VARCHAR(255),
  `PassWord` VARCHAR(255),
  `PersonalProfile` VARCHAR(255),
  `School` VARCHAR(255),
  `Major` VARCHAR(255),
  `JoinTime` VARCHAR(255),
  `CommentLikes` TEXT,
  `Solves` TEXT,
  `ACNum` INT,
  `SubmitNum` INT,
  `Authority` INT
);

CREATE TABLE `Problem` (
  `_id` INT PRIMARY KEY,
  `Title` VARCHAR(255),
  `Description` VARCHAR(255),
  `JudgeNum` INT,
  `SubmitNum` INT,
  `CENum` INT,
  `ACNum` INT,
  `WANum` INT,
  `RENum` INT,
  `TLENum` INT,
  `MLENum` INT,
  `SENum` INT,
  `Tags` TEXT
);

CREATE TABLE `StatusRecord` (
  `_id` BIGINT PRIMARY KEY,
  `ProblemId` BIGINT,
  `UserId` BIGINT,
  `UserNickName` VARCHAR(255),
  `ProblemTitle` VARCHAR(255),
  `Status` INT,
  `RunTime` VARCHAR(255),
  `RunMemory` VARCHAR(255),
  `Length` VARCHAR(255),
  `Language` VARCHAR(255),
  `SubmitTime` VARCHAR(255),
  `Code` TEXT,
  `CompilerInfo` TEXT
);

CREATE TABLE `Solution` (
  `_id` BIGINT PRIMARY KEY,
  `Title` VARCHAR(255),
  `ParentId` BIGINT,
  `Content` TEXT,
  `UserId` BIGINT,
  `Views` INT,
  `Comments` INT,
  `CreateTime` DATETIME,
  `Public` TINYINT(1),
  `UpdateTime` DATETIME
);

CREATE TABLE `Discuss` (
  `_id` BIGINT PRIMARY KEY,
  `Title` VARCHAR(255),
  `Content` TEXT,
  `ParentId` BIGINT,
  `UserId` BIGINT,
  `Views` INT,
  `Comments` INT,
  `CreateTime` DATETIME,
  `UpdateTime` DATETIME
);

CREATE TABLE `Announcement` (
  `_id` BIGINT PRIMARY KEY,
  `Title` VARCHAR(255),
  `Content` TEXT,
  `UserId` BIGINT,
  `Views` INT,
  `Comments` INT,
  `Level` INT,
  `CreateTime` DATETIME,
  `UpdateTime` DATETIME
);

CREATE TABLE `Comment` (
  `_id` BIGINT PRIMARY KEY,
  `ParentId` BIGINT,
  `ParentType` VARCHAR(255),
  `Content` TEXT,
  `UserId` BIGINT,
  `Likes` INT,
  `CreateTime` DATETIME
);

CREATE TABLE `Child_Comments` (
  `_id` BIGINT PRIMARY KEY,
  `Content` TEXT,
  `UserId` BIGINT,
  `Likes` INT,
  `CreateTime` DATETIME,
  `ParentCommentId` BIGINT,
  CONSTRAINT `FK_ParentComment` FOREIGN KEY (`ParentCommentId`)
    REFERENCES `Comment`(`_id`)
    ON DELETE CASCADE
);
