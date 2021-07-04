package main;

public class Candidate {
	private String name;
	private String surname;
	private String partyName;
	private int numOfDistrict;
	private int positionOnList;
	private int[] attributes;
	private int receivedVotes;
	
	public Candidate(String name, String surname, String partyName, int district,
					 int positionOnList, int[] attributes) {
		this.name = name;
		this.surname = surname;
		this.partyName = partyName;
		this.numOfDistrict = district;
		this.positionOnList = positionOnList;
		this.attributes = attributes;
		this.receivedVotes = 0;
	}
	
	public String getName() {
		return this.name;
	}
	
	public String getSurname() {
		return this.surname;
	}
	
	public int getNumOfDistrict() {
		return this.numOfDistrict;
	}
	
	public String getPartyName() {
		return this.partyName;
	}
	
	public int getPositionOnList() {
		return this.positionOnList;
	}
	
	public int getValueOfAttribute(int numOfAttribute) {
		return this.attributes[numOfAttribute - 1];
	}
	
	public int[] getAttributes() {
		return this.attributes;
	}
	
	public int getReceivedVotes() {
		return this.receivedVotes;
	}
	
	
	public void setNumOfDistrict(int numOfDistrict) {
		this.numOfDistrict = numOfDistrict;
	}
	
	public void setPositionOnList(int positionOnList) {
		this.positionOnList = positionOnList;
	}
	
	public void addVote() {
		this.receivedVotes++;
	}
	
	public String toString() {
		String ret = "";
		ret += name + " " + surname + " ";
		ret += partyName + " " + positionOnList + " " + receivedVotes;
		return ret;
	}
}
