-- phpMyAdmin SQL Dump
-- version 4.6.6deb5
-- https://www.phpmyadmin.net/
--
-- Client :  localhost:3306
-- Généré le :  Ven 05 Novembre 2021 à 17:27
-- Version du serveur :  10.3.22-MariaDB-0+deb10u1
-- Version de PHP :  7.3.14-1~deb10u1

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Base de données :  `Surveillance`
--

-- --------------------------------------------------------

--
-- Structure de la table `ALERTE`
--

CREATE TABLE `ALERTE` (
  `IdAlert` int(11) NOT NULL,
  `DateH` char(150) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  `IdCapt` int(11) DEFAULT NULL,
  `IdCam` int(11) DEFAULT NULL,
  `description` text CHARACTER SET utf8 COLLATE utf8_bin DEFAULT NULL,
  `DateAlerte` date DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Contenu de la table `ALERTE`
--

INSERT INTO `ALERTE` (`IdAlert`, `DateH`, `IdCapt`, `IdCam`, `description`, `DateAlerte`) VALUES
(2249, 'Wed Jul  7 17:34:21 2021\n', NULL, NULL, 'Coupure de courant', '2021-07-07'),
(2250, 'Wed Jul  7 17:34:35 2021\n', NULL, NULL, 'Retour du courant', '2021-07-07'),
(2277, 'Fri Nov  5 11:14:27 2021\n', 45, NULL, 'Mvt St Denis Entree', '2021-11-05');

-- --------------------------------------------------------

--
-- Structure de la table `CAMERA`
--

CREATE TABLE `CAMERA` (
  `IdCam` int(11) NOT NULL,
  `Nom` char(20) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  `NomRep` char(20) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  `Int_Ext` enum('EXT','INT') CHARACTER SET utf8 COLLATE utf8_bin DEFAULT NULL,
  `PosGeo` char(30) CHARACTER SET utf8 COLLATE utf8_bin DEFAULT NULL,
  `HautPX` int(11) NOT NULL,
  `LargPX` int(11) NOT NULL,
  `IP` char(15) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  `TypeAlert` enum('FTP','RTSP') CHARACTER SET utf8 COLLATE utf8_bin DEFAULT NULL,
  `NbPixAlert` int(11) DEFAULT NULL,
  `RepAlert` char(50) CHARACTER SET utf8 COLLATE utf8_bin DEFAULT NULL,
  `AdRTSP` char(50) CHARACTER SET utf8 COLLATE utf8_bin DEFAULT NULL,
  `ID` char(100) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Contenu de la table `CAMERA`
--

INSERT INTO `CAMERA` (`IdCam`, `Nom`, `NomRep`, `Int_Ext`, `PosGeo`, `HautPX`, `LargPX`, `IP`, `TypeAlert`, `NbPixAlert`, `RepAlert`, `AdRTSP`, `ID`) VALUES
(1, 'Cam51', 'cam51', 'INT', 'pos51', 720, 1280, '192.168.0.51', 'FTP', NULL, NULL, 'rtsp://192.168.0.51/11', 'WXH-121433-DFEAA'),
(2, 'Cam52', 'cam52', 'INT', 'pos52', 720, 1280, '192.168.0.52', 'FTP', NULL, NULL, NULL, ''),
(3, 'Cam53', 'cam53', 'EXT', 'pos53', 1280, 768, '192.168.0.53', 'RTSP', NULL, NULL, NULL, ''),
(4, 'Cam54', 'cam54', 'INT', 'pos54', 720, 1280, '192.168.0.54', 'RTSP', NULL, NULL, NULL, ''),
(5, 'Cam55', 'cam55', 'INT', 'Labo', 1024, 768, '192.168.0.55', 'FTP', NULL, NULL, NULL, ''),
(6, 'Cam56', 'cam56', 'INT', 'pos56', 1024, 768, '192.168.0.56', 'FTP', NULL, NULL, NULL, ''),
(7, 'Cam57', 'cam57', 'EXT', 'pos57-Atelier', 1024, 768, '192.168.0.57', 'FTP', NULL, NULL, NULL, '');

-- --------------------------------------------------------

--
-- Structure de la table `CAPTEUR`
--

CREATE TABLE `CAPTEUR` (
  `IdCapt` int(11) NOT NULL,
  `Type` char(20) CHARACTER SET utf8 COLLATE utf8_bin DEFAULT 'Température',
  `Nom` char(30) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  `PosGeo` char(60) CHARACTER SET utf8 COLLATE utf8_bin DEFAULT NULL,
  `adIP` char(20) CHARACTER SET utf8 COLLATE utf8_bin DEFAULT NULL,
  `Code` int(11) NOT NULL,
  `Valeur` float DEFAULT NULL,
  `alert` float DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Contenu de la table `CAPTEUR`
--

INSERT INTO `CAPTEUR` (`IdCapt`, `Type`, `Nom`, `PosGeo`, `adIP`, `Code`, `Valeur`, `alert`) VALUES
(35, 'Inconnu', 'Element inconnu', NULL, NULL, 0, NULL, NULL),
(36, 'Temperature', 'Capt Temp23', 'WC', NULL, 23, 99.9, 2),
(37, 'Ouverture', 'PF Salle Beutte', 'PF Salle', NULL, 12866900, NULL, NULL),
(38, 'Ouverture', 'Démo', 'Pas placé', NULL, 12866837, NULL, NULL),
(39, 'Mouvement', 'Capt Mvt Salle', 'Salle Beutte', NULL, 12801373, NULL, NULL),
(40, 'Ouverture', 'PorteEntree Beutte', 'Entrée', NULL, 12866901, NULL, NULL),
(41, 'Mouvement', 'Capt Mvt Chamb2', 'Chamb2 Beutte', NULL, 12801361, NULL, NULL),
(42, 'Mouvement', 'Capt Mvt Atelier', 'Atelier Beutte', NULL, 12801877, NULL, NULL),
(45, 'Mouvement', 'Mvt St Denis Entree', 'Entrée', NULL, 12801493, NULL, NULL),
(46, 'Temperature', 'Capt Temp21', 'Atelier', NULL, 21, 99.9, 2),
(48, 'Ouverture', 'Cave St Denis', 'Cuisine bas', NULL, 12867029, NULL, NULL),
(50, 'Mouvement', 'Capt Mvt Demo', 'Pas placé', NULL, 12770345, NULL, NULL),
(51, 'Temperature', 'Capt Temp24', 'Cuisine', NULL, 24, 99.9, 2),
(52, 'Temperature', 'Capt Temp25', 'Temp25', NULL, 25, 99.9, 2),
(53, 'Temperature', 'Capt Temp22', 'Temp22', NULL, 22, 99.9, 2);

-- --------------------------------------------------------

--
-- Structure de la table `IMAGE`
--

CREATE TABLE `IMAGE` (
  `idImage` int(11) NOT NULL,
  `nomImage` char(254) COLLATE utf8_bin DEFAULT NULL,
  `dateIm` datetime DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

--
-- Contenu de la table `IMAGE`
--

INSERT INTO `IMAGE` (`idImage`, `nomImage`, `dateIm`) VALUES
(0, NULL, NULL),
(1, 'cam51-01oct2021_14:00:40.jpg', '2021-10-01 14:00:40'),
(2, 'cam51-11oct2021_05:14:26.jpg', '2021-10-11 05:14:27'),
(3, 'cam51-11oct2021_10:17:30.jpg', '2021-10-11 10:17:30'),
(4, 'cam51-11oct2021_10:35:39.jpg', '2021-10-11 10:35:39'),
(5, 'cam51-11oct2021_12:01:25.jpg', '2021-10-11 12:01:25'),
(6, 'cam51-11oct2021_12:38:15.jpg', '2021-10-11 12:38:15'),
(7, 'cam51-13sept2021_09:19:25.jpg', '2021-09-13 09:19:25'),
(8, 'cam51-19sept2021_12:23:38.jpg', '2021-09-19 12:23:38'),
(9, 'cam51-19sept2021_12:24:16.jpg', '2021-09-19 12:24:16'),
(10, 'cam51-19sept2021_12:25:24.jpg', '2021-09-19 12:25:24'),
(11, 'cam51-19sept2021_12:28:17.jpg', '2021-09-19 12:28:17'),
(12, '6', NULL),
(13, NULL, NULL),
(14, NULL, NULL),
(15, NULL, NULL),
(16, NULL, NULL),
(17, NULL, NULL),
(18, NULL, NULL),
(19, NULL, NULL),
(20, NULL, NULL);

-- --------------------------------------------------------

--
-- Structure de la table `INFO`
--

CREATE TABLE `INFO` (
  `IdInfo` int(11) NOT NULL,
  `NoTel3G` char(13) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  `nomSite` char(30) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  `pwd` text CHARACTER SET utf8 COLLATE utf8_bin DEFAULT NULL,
  `state` int(11) DEFAULT NULL,
  `etatVMC` tinyint(1) DEFAULT 0,
  `pwdSMS` char(10) CHARACTER SET utf8 COLLATE utf8_bin DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Contenu de la table `INFO`
--

INSERT INTO `INFO` (`IdInfo`, `NoTel3G`, `nomSite`, `pwd`, `state`, `etatVMC`, `pwdSMS`) VALUES
(1, '0769813456', 'LaBeutte', 'test', 1, 0, '3456');

-- --------------------------------------------------------

--
-- Structure de la table `OPERATEUR`
--

CREATE TABLE `OPERATEUR` (
  `IdOp` int(11) NOT NULL,
  `login` char(20) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  `mdpasse` char(30) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  `IdPers` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Contenu de la table `OPERATEUR`
--

INSERT INTO `OPERATEUR` (`IdOp`, `login`, `mdpasse`, `IdPers`) VALUES
(1, 'admin', 'admin', 3),
(2, 'gilles', 'gilles', 3);

-- --------------------------------------------------------

--
-- Structure de la table `PERSONNE`
--

CREATE TABLE `PERSONNE` (
  `IdPers` int(11) NOT NULL,
  `Nom` char(30) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  `Prenom` char(30) CHARACTER SET utf8 COLLATE utf8_bin DEFAULT NULL,
  `Tel` char(13) CHARACTER SET utf8 COLLATE utf8_bin NOT NULL,
  `Mail` char(60) CHARACTER SET utf8 COLLATE utf8_bin DEFAULT NULL,
  `Groupe` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Contenu de la table `PERSONNE`
--

INSERT INTO `PERSONNE` (`IdPers`, `Nom`, `Prenom`, `Tel`, `Mail`, `Groupe`) VALUES
(1, 'Aucaigne', 'Gilles', '+33651980787', 'gillesaucaigne@orange.fr', 1),
(3, 'Labeutte', 'Gil', '+33651980787', 'gillesaucaigne@orange.fr', 2),
(4, 'Aucaigne', 'Patrick', '+33618512827', 'aucaignepatrick@orange.fr', 2);

--
-- Index pour les tables exportées
--

--
-- Index pour la table `ALERTE`
--
ALTER TABLE `ALERTE`
  ADD PRIMARY KEY (`IdAlert`),
  ADD UNIQUE KEY `ID_Alerte_IND` (`IdAlert`),
  ADD KEY `FKCap_Declenche` (`IdCapt`),
  ADD KEY `FKCam_Declenche` (`IdCam`);

--
-- Index pour la table `CAMERA`
--
ALTER TABLE `CAMERA`
  ADD PRIMARY KEY (`IdCam`);

--
-- Index pour la table `CAPTEUR`
--
ALTER TABLE `CAPTEUR`
  ADD PRIMARY KEY (`IdCapt`);

--
-- Index pour la table `IMAGE`
--
ALTER TABLE `IMAGE`
  ADD PRIMARY KEY (`idImage`);

--
-- Index pour la table `INFO`
--
ALTER TABLE `INFO`
  ADD PRIMARY KEY (`IdInfo`);

--
-- Index pour la table `OPERATEUR`
--
ALTER TABLE `OPERATEUR`
  ADD PRIMARY KEY (`IdOp`),
  ADD KEY `FKInclue` (`IdPers`);

--
-- Index pour la table `PERSONNE`
--
ALTER TABLE `PERSONNE`
  ADD PRIMARY KEY (`IdPers`);

--
-- AUTO_INCREMENT pour les tables exportées
--

--
-- AUTO_INCREMENT pour la table `ALERTE`
--
ALTER TABLE `ALERTE`
  MODIFY `IdAlert` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2278;
--
-- AUTO_INCREMENT pour la table `CAMERA`
--
ALTER TABLE `CAMERA`
  MODIFY `IdCam` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=57;
--
-- AUTO_INCREMENT pour la table `CAPTEUR`
--
ALTER TABLE `CAPTEUR`
  MODIFY `IdCapt` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=54;
--
-- AUTO_INCREMENT pour la table `INFO`
--
ALTER TABLE `INFO`
  MODIFY `IdInfo` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=2;
--
-- AUTO_INCREMENT pour la table `OPERATEUR`
--
ALTER TABLE `OPERATEUR`
  MODIFY `IdOp` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=3;
--
-- AUTO_INCREMENT pour la table `PERSONNE`
--
ALTER TABLE `PERSONNE`
  MODIFY `IdPers` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=5;
--
-- Contraintes pour les tables exportées
--

--
-- Contraintes pour la table `ALERTE`
--
ALTER TABLE `ALERTE`
  ADD CONSTRAINT `FKCam_Declenche` FOREIGN KEY (`IdCam`) REFERENCES `CAMERA` (`IdCam`),
  ADD CONSTRAINT `FKCap_Declenche` FOREIGN KEY (`IdCapt`) REFERENCES `CAPTEUR` (`IdCapt`);

--
-- Contraintes pour la table `OPERATEUR`
--
ALTER TABLE `OPERATEUR`
  ADD CONSTRAINT `FKInclue` FOREIGN KEY (`IdPers`) REFERENCES `PERSONNE` (`IdPers`);

DELIMITER $$
--
-- Événements
--
CREATE DEFINER=`root`@`localhost` EVENT `cleanAlerte` ON SCHEDULE EVERY 1 MONTH STARTS '2021-09-01 00:00:00' ON COMPLETION PRESERVE ENABLE DO BEGIN
   DELETE FROM ALERTE 
   WHERE DateAlerte < (CURRENT_DATE - INTERVAL 92 DAY);
END$$

DELIMITER ;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
