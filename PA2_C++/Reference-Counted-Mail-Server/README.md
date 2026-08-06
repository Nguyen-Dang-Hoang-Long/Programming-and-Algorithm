Reference-Counted-Mail-Server

Develop classes CMailbox and CMail which simulates a containers of mails and a mail respectively.

They need to be used in tandem with already pre-implemented classes such as CTimeStamp (represents a date), CMailBody (mail body), CAttach (mail attachment )

Both classes requires constructor, destructor, copy constructor, assignment operator=.

Other designed methods:
CMailbox:
- delivery (mail): Called when a new e-mail is to be stored into the mailbox. The new mail is copied into the inbox folder. 
- newFolder (name): Creates a new folder named name.
- moveMail (fromFld, toFld): Moves all e-mails from folder fromFld into folder toFld.
- listMail (fld, from, to): Returns a list of e-mails selected from folder fld if the e-mail time stamp fits the time interval (from - to, both inclusive).
- listAddr (from, to): Returns a set of e-mail addresses from e-mail that fits into the interval (from - to, both inclusive).

CMail:
- Getters for individual fields (time stamp, sender, mail body, attachment).
- Operator <<: Displays the time stamp, sender, mail body and (optionally) the attachment.

Binary searches, O(1) shallow copy, reference counting, and deep copy is used.