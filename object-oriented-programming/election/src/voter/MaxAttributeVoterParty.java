package voter;

import main.Candidate;
import main.Party;

import java.util.List;
import java.util.Map;

/* Maksymalizujący jednocechowy wybierający spośród jednej partii. */
public class MaxAttributeVoterParty extends MaxAttributeVoter {
	protected Party party;
	
	public MaxAttributeVoterParty(String name, String surname, int numOfDistrict,
								  int numOfAttribute, Party party) {
		super(name, surname, numOfDistrict, numOfAttribute);
		this.party = party;
	}
	
	@Override
	public void vote(Map<Party, List<Candidate>> ticket) {
		super.iterateOverPartyList(ticket.get(party));
		this.chosenCandidate.addVote();
	}
}
