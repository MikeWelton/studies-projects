package voter;

import main.Candidate;
import main.Party;

import java.util.List;
import java.util.Map;

/* Minimalizujący jednocechowy wybierający spośród jednej partii. */
public class MiniAttributeVoterParty extends MiniAttributeVoter {
	protected Party party;
	
	public MiniAttributeVoterParty(String name, String surname, int numOfDistrict,
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
